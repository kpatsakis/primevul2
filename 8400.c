bool read_pred_weight_table(bitreader* br, slice_segment_header* shdr, decoder_context* ctx)
{
  int vlc;

  pic_parameter_set* pps = ctx->get_pps((int)shdr->slice_pic_parameter_set_id);
  assert(pps);
  seq_parameter_set* sps = ctx->get_sps((int)pps->seq_parameter_set_id);
  assert(sps);

  shdr->luma_log2_weight_denom = vlc = get_uvlc(br);
  if (vlc<0 || vlc>7) return false;

  if (sps->chroma_format_idc != 0) {
    vlc = get_svlc(br);
    vlc += shdr->luma_log2_weight_denom;
    if (vlc<0 || vlc>7) return false;
    shdr->ChromaLog2WeightDenom = vlc;
  }

  int sumWeightFlags = 0;

  for (int l=0;l<=1;l++)
    if (l==0 || (l==1 && shdr->slice_type == SLICE_TYPE_B))
      {
        int num_ref = (l==0 ? shdr->num_ref_idx_l0_active-1 : shdr->num_ref_idx_l1_active-1);

        for (int i=0;i<=num_ref;i++) {
          shdr->luma_weight_flag[l][i] = get_bits(br,1);
          if (shdr->luma_weight_flag[l][i]) sumWeightFlags++;
        }

        if (sps->chroma_format_idc != 0) {
          for (int i=0;i<=num_ref;i++) {
            shdr->chroma_weight_flag[l][i] = get_bits(br,1);
            if (shdr->chroma_weight_flag[l][i]) sumWeightFlags+=2;
          }
        }

        for (int i=0;i<=num_ref;i++) {
          if (shdr->luma_weight_flag[l][i]) {

            // delta_luma_weight

            vlc = get_svlc(br);
            if (vlc < -128 || vlc > 127) return false;

            shdr->LumaWeight[l][i] = (1<<shdr->luma_log2_weight_denom) + vlc;

            // luma_offset

            vlc = get_svlc(br);
            if (vlc < -sps->WpOffsetHalfRangeY || vlc > sps->WpOffsetHalfRangeY-1) return false;
            shdr->luma_offset[l][i] = vlc;
          }
          else {
            shdr->LumaWeight[l][i] = 1<<shdr->luma_log2_weight_denom;
            shdr->luma_offset[l][i] = 0;
          }

          if (shdr->chroma_weight_flag[l][i])
            for (int j=0;j<2;j++) {
              // delta_chroma_weight

              vlc = get_svlc(br);
              if (vlc < -128 || vlc >  127) return false;

              shdr->ChromaWeight[l][i][j] = (1<<shdr->ChromaLog2WeightDenom) + vlc;

              // delta_chroma_offset

              vlc = get_svlc(br);
              if (vlc < -4*sps->WpOffsetHalfRangeC ||
                  vlc >  4*sps->WpOffsetHalfRangeC-1) return false;

              vlc = Clip3(-sps->WpOffsetHalfRangeC,
                          sps->WpOffsetHalfRangeC-1,
                          (sps->WpOffsetHalfRangeC
                           +vlc
                           -((sps->WpOffsetHalfRangeC*shdr->ChromaWeight[l][i][j])
                             >> shdr->ChromaLog2WeightDenom)));

              shdr->ChromaOffset[l][i][j] = vlc;
            }
          else {
            for (int j=0;j<2;j++) {
              shdr->ChromaWeight[l][i][j] = 1<<shdr->ChromaLog2WeightDenom;
              shdr->ChromaOffset[l][i][j] = 0;
            }
          }
        }
      }

  // TODO: bitstream conformance requires that 'sumWeightFlags<=24'

  return true;
}