static int decode_ref_idx_lX(thread_context* tctx, int numRefIdxLXActive)
{
  logtrace(LogSlice,"# ref_idx_lX\n");

  int cMax = numRefIdxLXActive-1;

  if (cMax==0) {
    logtrace(LogSlice,"> ref_idx = 0 (cMax==0)\n");
    return 0;
  } // do check for single reference frame here

  int bit = decode_CABAC_bit(&tctx->cabac_decoder,
                             &tctx->ctx_model[CONTEXT_MODEL_REF_IDX_LX + 0]);

  int idx=0;

  while (bit) {
    idx++;
    if (idx==cMax) { break; }

    if (idx==1) {
      bit = decode_CABAC_bit(&tctx->cabac_decoder,
                             &tctx->ctx_model[CONTEXT_MODEL_REF_IDX_LX + 1]);
    }
    else {
      bit = decode_CABAC_bypass(&tctx->cabac_decoder);
    }
  }

  logtrace(LogSlice,"> ref_idx = %d\n",idx);

  logtrace(LogSymbols,"$1 ref_idx_lX=%d\n",idx);
  return idx;
}