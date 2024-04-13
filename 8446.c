void thread_task_slice_segment::work()
{
  thread_task_slice_segment* data = this;
  thread_context* tctx = data->tctx;
  de265_image* img = tctx->img;

  state = Running;
  img->thread_run(this);

  setCtbAddrFromTS(tctx);

  //printf("%p: A start decoding at %d/%d\n", tctx, tctx->CtbX,tctx->CtbY);

  if (data->firstSliceSubstream) {
    bool success = initialize_CABAC_at_slice_segment_start(tctx);
    if (!success) {
      state = Finished;
      tctx->sliceunit->finished_threads.increase_progress(1);
      img->thread_finishes(this);
      return;
    }
  }
  else {
    initialize_CABAC_models(tctx);
  }

  init_CABAC_decoder_2(&tctx->cabac_decoder);

  /*enum DecodeResult result =*/ decode_substream(tctx, false, data->firstSliceSubstream);

  state = Finished;
  tctx->sliceunit->finished_threads.increase_progress(1);
  img->thread_finishes(this);

  return; // DE265_OK;
}