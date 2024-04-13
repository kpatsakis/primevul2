void read_sao(thread_context* tctx, int xCtb,int yCtb,
              int CtbAddrInSliceSeg)
{
  slice_segment_header* shdr = tctx->shdr;
  de265_image* img = tctx->img;
  const seq_parameter_set& sps = img->get_sps();
  const pic_parameter_set& pps = img->get_pps();

  logtrace(LogSlice,"# read_sao(%d,%d)\n",xCtb,yCtb);

  sao_info saoinfo;
  memset(&saoinfo,0,sizeof(sao_info));
  logtrace(LogSlice,"sizeof saoinfo: %d\n",sizeof(sao_info));


  char sao_merge_left_flag = 0;
  char sao_merge_up_flag = 0;

  if (xCtb>0) {
    //char leftCtbInSliceSeg = (CtbAddrInSliceSeg>0);
    char leftCtbInSliceSeg = (tctx->CtbAddrInRS > shdr->SliceAddrRS);
    char leftCtbInTile = (pps.TileIdRS[xCtb   + yCtb * sps.PicWidthInCtbsY] ==
                          pps.TileIdRS[xCtb-1 + yCtb * sps.PicWidthInCtbsY]);

    if (leftCtbInSliceSeg && leftCtbInTile) {
      sao_merge_left_flag = decode_sao_merge_flag(tctx);
      logtrace(LogSlice,"sao_merge_left_flag: %d\n",sao_merge_left_flag);
    }
  }

  if (yCtb>0 && sao_merge_left_flag==0) {
    logtrace(LogSlice,"CtbAddrInRS:%d PicWidthInCtbsY:%d slice_segment_address:%d\n",
             tctx->CtbAddrInRS,
             sps.PicWidthInCtbsY,
             shdr->slice_segment_address);
    char upCtbInSliceSeg = (tctx->CtbAddrInRS - sps.PicWidthInCtbsY) >= shdr->SliceAddrRS;
    char upCtbInTile = (pps.TileIdRS[xCtb +  yCtb    * sps.PicWidthInCtbsY] ==
                        pps.TileIdRS[xCtb + (yCtb-1) * sps.PicWidthInCtbsY]);

    if (upCtbInSliceSeg && upCtbInTile) {
      sao_merge_up_flag = decode_sao_merge_flag(tctx);
      logtrace(LogSlice,"sao_merge_up_flag: %d\n",sao_merge_up_flag);
    }
  }

  if (!sao_merge_up_flag && !sao_merge_left_flag) {
    int nChroma = 3;
    if (sps.ChromaArrayType == CHROMA_MONO) nChroma=1;

    for (int cIdx=0; cIdx<nChroma; cIdx++) {
      if ((shdr->slice_sao_luma_flag && cIdx==0) ||
          (shdr->slice_sao_chroma_flag && cIdx>0)) {

        uint8_t SaoTypeIdx = 0;

        if (cIdx==0) {
          char sao_type_idx_luma = decode_sao_type_idx(tctx);
          logtrace(LogSlice,"sao_type_idx_luma: %d\n", sao_type_idx_luma);
          saoinfo.SaoTypeIdx = SaoTypeIdx = sao_type_idx_luma;
        }
        else if (cIdx==1) {
          char sao_type_idx_chroma = decode_sao_type_idx(tctx);
          logtrace(LogSlice,"sao_type_idx_chroma: %d\n", sao_type_idx_chroma);
          SaoTypeIdx = sao_type_idx_chroma;
          saoinfo.SaoTypeIdx |= SaoTypeIdx<<(2*1);
          saoinfo.SaoTypeIdx |= SaoTypeIdx<<(2*2);  // set for both chroma components
        }
        else {
          // SaoTypeIdx = 0

          SaoTypeIdx = (saoinfo.SaoTypeIdx >> (2*cIdx)) & 0x3;
        }

        if (SaoTypeIdx != 0) {
          for (int i=0;i<4;i++) {
            saoinfo.saoOffsetVal[cIdx][i] = decode_sao_offset_abs(tctx, img->get_bit_depth(cIdx));
            logtrace(LogSlice,"saoOffsetVal[%d][%d] = %d\n",cIdx,i, saoinfo.saoOffsetVal[cIdx][i]);
          }

          int sign[4];
          if (SaoTypeIdx==1) {
            for (int i=0;i<4;i++) {
              if (saoinfo.saoOffsetVal[cIdx][i] != 0) {
                sign[i] = decode_sao_offset_sign(tctx) ? -1 : 1;
              }
              else {
                sign[i] = 0; // not really required, but compiler warns about uninitialized values
              }
            }

            saoinfo.sao_band_position[cIdx] = decode_sao_band_position(tctx);
          }
          else {
            uint8_t SaoEoClass = 0;

            sign[0] = sign[1] =  1;
            sign[2] = sign[3] = -1;

            if (cIdx==0) {
              saoinfo.SaoEoClass = SaoEoClass = decode_sao_class(tctx);
            }
            else if (cIdx==1) {
              SaoEoClass = decode_sao_class(tctx);
              saoinfo.SaoEoClass |= SaoEoClass << (2*1);
              saoinfo.SaoEoClass |= SaoEoClass << (2*2);
            }

            logtrace(LogSlice,"SaoEoClass[%d] = %d\n",cIdx,SaoEoClass);
          }

          int log2OffsetScale;

          if (cIdx==0) {
            log2OffsetScale = pps.range_extension.log2_sao_offset_scale_luma;
          }
          else {
            log2OffsetScale = pps.range_extension.log2_sao_offset_scale_chroma;
          }

          for (int i=0;i<4;i++) {
            saoinfo.saoOffsetVal[cIdx][i] = sign[i]*(saoinfo.saoOffsetVal[cIdx][i] << log2OffsetScale);
          }
        }
      }
    }

    img->set_sao_info(xCtb,yCtb,  &saoinfo);
  }


  if (sao_merge_left_flag) {
    img->set_sao_info(xCtb,yCtb,  img->get_sao_info(xCtb-1,yCtb));
  }

  if (sao_merge_up_flag) {
    img->set_sao_info(xCtb,yCtb,  img->get_sao_info(xCtb,yCtb-1));
  }
}