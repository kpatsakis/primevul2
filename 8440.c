static int decode_merge_idx(thread_context* tctx)
{
  logtrace(LogSlice,"# merge_idx\n");

  if (tctx->shdr->MaxNumMergeCand <= 1) {
    logtrace(LogSymbols,"$1 merge_idx=%d\n",0);
    return 0;
  }

  // TU coding, first bin is CABAC, remaining are bypass.
  // cMax = MaxNumMergeCand-1

  int idx = decode_CABAC_bit(&tctx->cabac_decoder,
                             &tctx->ctx_model[CONTEXT_MODEL_MERGE_IDX]);

  if (idx==0) {
    // nothing
  }
  else {
    idx=1;

    while (idx<tctx->shdr->MaxNumMergeCand-1) {
      if (decode_CABAC_bypass(&tctx->cabac_decoder)) {
        idx++;
      }
      else {
        break;
      }
    }
  }

  logtrace(LogSlice,"> merge_idx = %d\n",idx);
  logtrace(LogSymbols,"$1 merge_idx=%d\n",idx);

  return idx;
}