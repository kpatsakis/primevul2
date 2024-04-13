de265_error read_slice_segment_data(thread_context* tctx)
{
  setCtbAddrFromTS(tctx);

  de265_image* img = tctx->img;
  const pic_parameter_set& pps = img->get_pps();
  const seq_parameter_set& sps = img->get_sps();
  slice_segment_header* shdr = tctx->shdr;

  bool success = initialize_CABAC_at_slice_segment_start(tctx);
  if (!success) {
    return DE265_ERROR_UNSPECIFIED_DECODING_ERROR;
  }

  init_CABAC_decoder_2(&tctx->cabac_decoder);

  //printf("-----\n");

  bool first_slice_substream = !shdr->dependent_slice_segment_flag;

  int substream=0;

  enum DecodeResult result;
  do {
    int ctby = tctx->CtbY;


    // check whether entry_points[] are correct in the bitstream

    if (substream>0) {
      if (substream-1 >= tctx->shdr->entry_point_offset.size() ||
          tctx->cabac_decoder.bitstream_curr - tctx->cabac_decoder.bitstream_start -2 /* -2 because of CABAC init */
          != tctx->shdr->entry_point_offset[substream-1]) {
        tctx->decctx->add_warning(DE265_WARNING_INCORRECT_ENTRY_POINT_OFFSET, true);
      }
    }

    substream++;


    result = decode_substream(tctx, false, first_slice_substream);


    if (result == Decode_EndOfSliceSegment ||
        result == Decode_Error) {
      break;
    }

    first_slice_substream = false;

    if (pps.tiles_enabled_flag) {
      initialize_CABAC_models(tctx);
    }
  } while (true);

  return DE265_OK;
}