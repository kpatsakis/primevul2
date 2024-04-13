void thread_task_ctb_row::work()
{
  thread_task_ctb_row* data = this;
  thread_context* tctx = data->tctx;
  de265_image* img = tctx->img;

  const seq_parameter_set& sps = img->get_sps();
  int ctbW = sps.PicWidthInCtbsY;

  state = Running;
  img->thread_run(this);

  setCtbAddrFromTS(tctx);

  int ctby = tctx->CtbAddrInRS / ctbW;
  int myCtbRow = ctby;

  //printf("start CTB-row decoding at row %d\n", ctby);

  if (data->firstSliceSubstream) {
    bool success = initialize_CABAC_at_slice_segment_start(tctx);
    if (!success) {
      // could not decode this row, mark whole row as finished
      for (int x=0;x<ctbW;x++) {
        img->ctb_progress[myCtbRow*ctbW + x].set_progress(CTB_PROGRESS_PREFILTER);
      }

      state = Finished;
      tctx->sliceunit->finished_threads.increase_progress(1);
      img->thread_finishes(this);
      return;
    }
    //initialize_CABAC(tctx);
  }

  init_CABAC_decoder_2(&tctx->cabac_decoder);

  bool firstIndependentSubstream =
    data->firstSliceSubstream && !tctx->shdr->dependent_slice_segment_flag;

  /*enum DecodeResult result =*/
  decode_substream(tctx, true, firstIndependentSubstream);

  // mark progress on remaining CTBs in row (in case of decoder error and early termination)

  // TODO: what about slices that end properly in the middle of a CTB row?

  if (tctx->CtbY == myCtbRow) {
    int lastCtbX = sps.PicWidthInCtbsY; // assume no tiles when WPP is on
    for (int x = tctx->CtbX; x<lastCtbX ; x++) {

      if (x        < sps.PicWidthInCtbsY &&
          myCtbRow < sps.PicHeightInCtbsY) {
        img->ctb_progress[myCtbRow*ctbW + x].set_progress(CTB_PROGRESS_PREFILTER);
      }
    }
  }

  state = Finished;
  tctx->sliceunit->finished_threads.increase_progress(1);
  img->thread_finishes(this);
}