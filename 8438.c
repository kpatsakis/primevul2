static inline int decode_coded_sub_block_flag(thread_context* tctx,
                                              int cIdx,
                                              uint8_t coded_sub_block_neighbors)
{
  logtrace(LogSlice,"# coded_sub_block_flag\n");

  // tricky computation of csbfCtx
  int csbfCtx = ((coded_sub_block_neighbors &  1) |  // right neighbor set  or
                 (coded_sub_block_neighbors >> 1));  // bottom neighbor set   -> csbfCtx=1

  int ctxIdxInc = csbfCtx;
  if (cIdx!=0) {
    ctxIdxInc += 2;
  }

  int bit = decode_CABAC_bit(&tctx->cabac_decoder,
                             &tctx->ctx_model[CONTEXT_MODEL_CODED_SUB_BLOCK_FLAG + ctxIdxInc]);

  logtrace(LogSymbols,"$1 coded_sub_block_flag=%d\n",bit);
  return bit;
}