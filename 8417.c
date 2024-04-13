bool initialize_CABAC_at_slice_segment_start(thread_context* tctx)
{
  de265_image* img = tctx->img;
  const pic_parameter_set& pps = img->get_pps();
  const seq_parameter_set& sps = img->get_sps();
  slice_segment_header* shdr = tctx->shdr;

  if (shdr->dependent_slice_segment_flag) {
    int prevCtb = pps.CtbAddrTStoRS[ pps.CtbAddrRStoTS[shdr->slice_segment_address] -1 ];

    int sliceIdx = img->get_SliceHeaderIndex_atIndex(prevCtb);
    if (sliceIdx >= img->slices.size()) {
      return false;
    }
    slice_segment_header* prevCtbHdr = img->slices[ sliceIdx ];

    if (pps.is_tile_start_CTB(shdr->slice_segment_address % sps.PicWidthInCtbsY,
                              shdr->slice_segment_address / sps.PicWidthInCtbsY
                              )) {
      initialize_CABAC_models(tctx);
    }
    else {
      // wait for previous slice to finish decoding

      //printf("wait for previous slice to finish decoding\n");


      slice_unit* prevSliceSegment = tctx->imgunit->get_prev_slice_segment(tctx->sliceunit);
      //assert(prevSliceSegment);
      if (prevSliceSegment==NULL) {
        return false;
      }

      prevSliceSegment->finished_threads.wait_for_progress(prevSliceSegment->nThreads);


      /*
      printf("wait for %d,%d (init)\n",
             prevCtb / sps->PicWidthInCtbsY,
             prevCtb % sps->PicWidthInCtbsY);
      tctx->img->wait_for_progress(tctx->task, prevCtb, CTB_PROGRESS_PREFILTER);
      */

      if (!prevCtbHdr->ctx_model_storage_defined) {
        return false;
      }

      tctx->ctx_model = prevCtbHdr->ctx_model_storage;
      prevCtbHdr->ctx_model_storage.release();
    }
  }
  else {
    initialize_CABAC_models(tctx);
  }

  return true;
}