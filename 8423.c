static int decode_split_cu_flag(thread_context* tctx,
				int x0, int y0, int ctDepth)
{
  // check if neighbors are available

  int availableL = check_CTB_available(tctx->img, x0,y0, x0-1,y0);
  int availableA = check_CTB_available(tctx->img, x0,y0, x0,y0-1);

  int condL = 0;
  int condA = 0;

  if (availableL && tctx->img->get_ctDepth(x0-1,y0) > ctDepth) condL=1;
  if (availableA && tctx->img->get_ctDepth(x0,y0-1) > ctDepth) condA=1;

  int contextOffset = condL + condA;
  int context = contextOffset;

  // decode bit

  logtrace(LogSlice,"# split_cu_flag context=%d R=%x\n", context, tctx->cabac_decoder.range);

  int bit = decode_CABAC_bit(&tctx->cabac_decoder, &tctx->ctx_model[CONTEXT_MODEL_SPLIT_CU_FLAG + context]);

  logtrace(LogSlice,"> split_cu_flag R=%x, ctx=%d, bit=%d\n", tctx->cabac_decoder.range,context,bit);

  logtrace(LogSymbols,"$1 split_cu_flag=%d\n",bit);

  return bit;
}