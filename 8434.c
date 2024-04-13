enum DecodeResult decode_substream(thread_context* tctx,
                                   bool block_wpp, // block on WPP dependencies
                                   bool first_independent_substream)
{
  const pic_parameter_set& pps = tctx->img->get_pps();
  const seq_parameter_set& sps = tctx->img->get_sps();

  const int ctbW = sps.PicWidthInCtbsY;


  const int startCtbY = tctx->CtbY;

  //printf("start decoding substream at %d;%d\n",tctx->CtbX,tctx->CtbY);

  // in WPP mode: initialize CABAC model with stored model from row above

  if ((!first_independent_substream || tctx->CtbY != startCtbY) &&
      pps.entropy_coding_sync_enabled_flag &&
      tctx->CtbY>=1 && tctx->CtbX==0)
    {
      if (sps.PicWidthInCtbsY>1) {
        if ((tctx->CtbY-1) >= tctx->imgunit->ctx_models.size()) {
          return Decode_Error;
        }

        //printf("CTX wait on %d/%d\n",1,tctx->CtbY-1);

        // we have to wait until the context model data is there
        tctx->img->wait_for_progress(tctx->task, 1,tctx->CtbY-1,CTB_PROGRESS_PREFILTER);

        // copy CABAC model from previous CTB row
        tctx->ctx_model = tctx->imgunit->ctx_models[(tctx->CtbY-1)];
        tctx->imgunit->ctx_models[(tctx->CtbY-1)].release(); // not used anymore
      }
      else {
        tctx->img->wait_for_progress(tctx->task, 0,tctx->CtbY-1,CTB_PROGRESS_PREFILTER);
        initialize_CABAC_models(tctx);
      }
    }


  do {
    const int ctbx = tctx->CtbX;
    const int ctby = tctx->CtbY;

    if (ctbx+ctby*ctbW >= pps.CtbAddrRStoTS.size()) {
        return Decode_Error;
    }

    if (ctbx >= sps.PicWidthInCtbsY ||
        ctby >= sps.PicHeightInCtbsY) {
        return Decode_Error;
    }

    if (block_wpp && ctby>0 && ctbx < ctbW-1) {

      // TODO: if we are in tiles mode and at the right border, do not wait for x+1,y-1

      //printf("wait on %d/%d (%d)\n",ctbx+1,ctby-1, ctbx+1+(ctby-1)*sps->PicWidthInCtbsY);

      tctx->img->wait_for_progress(tctx->task, ctbx+1,ctby-1, CTB_PROGRESS_PREFILTER);
    }

    //printf("%p: decode %d;%d\n", tctx, tctx->CtbX,tctx->CtbY);


    // read and decode CTB

    if (tctx->ctx_model.empty() == false) {
      return Decode_Error;
    }

    read_coding_tree_unit(tctx);


    // save CABAC-model for WPP (except in last CTB row)

    if (pps.entropy_coding_sync_enabled_flag &&
        ctbx == 1 &&
        ctby < sps.PicHeightInCtbsY-1)
      {
        // no storage for context table has been allocated
        if (tctx->imgunit->ctx_models.size() <= ctby) {
          return Decode_Error;
        }

        tctx->imgunit->ctx_models[ctby] = tctx->ctx_model;
        tctx->imgunit->ctx_models[ctby].decouple(); // store an independent copy
      }


    // end of slice segment ?

    int end_of_slice_segment_flag = decode_CABAC_term_bit(&tctx->cabac_decoder);
    //printf("end-of-slice flag: %d\n", end_of_slice_segment_flag);

    if (end_of_slice_segment_flag) {
      // at the end of the slice segment, we store the CABAC model if we need it
      // because a dependent slice may follow

      if (pps.dependent_slice_segments_enabled_flag) {
        tctx->shdr->ctx_model_storage = tctx->ctx_model;
        tctx->shdr->ctx_model_storage.decouple(); // store an independent copy

        tctx->shdr->ctx_model_storage_defined = true;
      }
    }

    tctx->img->ctb_progress[ctbx+ctby*ctbW].set_progress(CTB_PROGRESS_PREFILTER);

    //printf("%p: decoded %d|%d\n",tctx, ctby,ctbx);


    logtrace(LogSlice,"read CTB %d -> end=%d\n", tctx->CtbAddrInRS, end_of_slice_segment_flag);
    //printf("read CTB %d -> end=%d\n", tctx->CtbAddrInRS, end_of_slice_segment_flag);

    const int lastCtbY = tctx->CtbY;

    bool endOfPicture = advanceCtbAddr(tctx); // true if we read past the end of the image

    if (endOfPicture &&
        end_of_slice_segment_flag == false)
      {
        tctx->decctx->add_warning(DE265_WARNING_CTB_OUTSIDE_IMAGE_AREA, false);
        tctx->img->integrity = INTEGRITY_DECODING_ERRORS;
        return Decode_Error;
      }


    if (end_of_slice_segment_flag) {
      /* corrupted inputs may send the end_of_slice_segment_flag even if not all
         CTBs in a row have been coded. Hence, we mark all of them as finished.
       */

      /*
      for (int x = ctbx+1 ; x<sps->PicWidthInCtbsY; x++) {
        printf("mark skipped %d;%d\n",ctbx,ctby);
        tctx->img->ctb_progress[ctbx+ctby*ctbW].set_progress(CTB_PROGRESS_PREFILTER);
      }
      */

      return Decode_EndOfSliceSegment;
    }


    if (!end_of_slice_segment_flag) {
      bool end_of_sub_stream = false;
      end_of_sub_stream |= (pps.tiles_enabled_flag &&
                            pps.TileId[tctx->CtbAddrInTS] != pps.TileId[tctx->CtbAddrInTS-1]);
      end_of_sub_stream |= (pps.entropy_coding_sync_enabled_flag &&
                            lastCtbY != tctx->CtbY);

      if (end_of_sub_stream) {
        int end_of_sub_stream_one_bit = decode_CABAC_term_bit(&tctx->cabac_decoder);
        if (!end_of_sub_stream_one_bit) {
          tctx->decctx->add_warning(DE265_WARNING_EOSS_BIT_NOT_SET, false);
          tctx->img->integrity = INTEGRITY_DECODING_ERRORS;
          return Decode_Error;
        }

        init_CABAC_decoder_2(&tctx->cabac_decoder); // byte alignment
        return Decode_EndOfSubstream;
      }
    }

  } while (true);
}