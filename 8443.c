static int decode_coeff_abs_level_greater2(thread_context* tctx,
					   int cIdx, // int i,int n,
					   int ctxSet)
{
  logtrace(LogSlice,"# coeff_abs_level_greater2\n");

  int ctxIdxInc = ctxSet;

  if (cIdx>0) ctxIdxInc+=4;

  int bit = decode_CABAC_bit(&tctx->cabac_decoder,
                             &tctx->ctx_model[CONTEXT_MODEL_COEFF_ABS_LEVEL_GREATER2_FLAG + ctxIdxInc]);

  logtrace(LogSymbols,"$1 coeff_abs_level_greater2=%d\n",bit);

  return bit;
}