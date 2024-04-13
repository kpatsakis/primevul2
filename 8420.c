static int decode_log2_res_scale_abs_plus1(thread_context* tctx, int cIdxMinus1)
{
  //const int context = (cIdx==0) ? 0 : 1;

  logtrace(LogSlice,"# log2_res_scale_abs_plus1 (c=%d)\n",cIdxMinus1);

  int value = 0;
  int cMax  = 4;
  for (int binIdx=0;binIdx<cMax;binIdx++)
    {
      int ctxIdxInc = 4*cIdxMinus1 + binIdx;

      int bit = decode_CABAC_bit(&tctx->cabac_decoder,
                                 &tctx->ctx_model[CONTEXT_MODEL_LOG2_RES_SCALE_ABS_PLUS1+ctxIdxInc]);
      if (!bit) break;
      value++;
    }

  logtrace(LogSymbols,"$1 log2_res_scale_abs_plus1=%d\n",value);

  return value;
}