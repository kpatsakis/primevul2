static int decode_res_scale_sign_flag(thread_context* tctx, int cIdxMinus1)
{
  //const int context = (cIdx==0) ? 0 : 1;

  logtrace(LogSlice,"# res_scale_sign_flag (c=%d)\n",cIdxMinus1);

  int bit = decode_CABAC_bit(&tctx->cabac_decoder,
                             &tctx->ctx_model[CONTEXT_MODEL_RES_SCALE_SIGN_FLAG+cIdxMinus1]);

  logtrace(LogSymbols,"$1 res_scale_sign_flag=%d\n",bit);

  return bit;
}