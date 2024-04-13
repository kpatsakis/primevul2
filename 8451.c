static inline int decode_coeff_abs_level_greater1(thread_context* tctx,
                                                  int cIdx, int i,
                                                  bool firstCoeffInSubblock,
                                                  bool firstSubblock,
                                                  int  lastSubblock_greater1Ctx,
                                                  int* lastInvocation_greater1Ctx,
                                                  int* lastInvocation_coeff_abs_level_greater1_flag,
                                                  int* lastInvocation_ctxSet, int c1)
{
  logtrace(LogSlice,"# coeff_abs_level_greater1\n");

  logtrace(LogSlice,"  cIdx:%d i:%d firstCoeffInSB:%d firstSB:%d lastSB>1:%d last>1Ctx:%d lastLev>1:%d lastCtxSet:%d\n", cIdx,i,firstCoeffInSubblock,firstSubblock,lastSubblock_greater1Ctx,
	   *lastInvocation_greater1Ctx,
	   *lastInvocation_coeff_abs_level_greater1_flag,
	   *lastInvocation_ctxSet);

  int lastGreater1Ctx;
  int greater1Ctx;
  int ctxSet;

  logtrace(LogSlice,"c1: %d\n",c1);

  if (firstCoeffInSubblock) {
    // block with real DC -> ctx 0
    if (i==0 || cIdx>0) { ctxSet=0; }
    else { ctxSet=2; }

    if (firstSubblock) { lastGreater1Ctx=1; }
    else { lastGreater1Ctx = lastSubblock_greater1Ctx; }

    if (lastGreater1Ctx==0) { ctxSet++; }

    logtrace(LogSlice,"ctxSet: %d\n",ctxSet);

    greater1Ctx=1;
  }
  else { // !firstCoeffInSubblock
    ctxSet = *lastInvocation_ctxSet;
    logtrace(LogSlice,"ctxSet (old): %d\n",ctxSet);

    greater1Ctx = *lastInvocation_greater1Ctx;
    if (greater1Ctx>0) {
      int lastGreater1Flag=*lastInvocation_coeff_abs_level_greater1_flag;
      if (lastGreater1Flag==1) greater1Ctx=0;
      else { /*if (greater1Ctx>0)*/ greater1Ctx++; }
    }
  }

  ctxSet = c1; // use HM algo

  int ctxIdxInc = (ctxSet*4) + (greater1Ctx>=3 ? 3 : greater1Ctx);

  if (cIdx>0) { ctxIdxInc+=16; }

  int bit = decode_CABAC_bit(&tctx->cabac_decoder,
                             &tctx->ctx_model[CONTEXT_MODEL_COEFF_ABS_LEVEL_GREATER1_FLAG + ctxIdxInc]);

  *lastInvocation_greater1Ctx = greater1Ctx;
  *lastInvocation_coeff_abs_level_greater1_flag = bit;
  *lastInvocation_ctxSet = ctxSet;

  //logtrace(LogSymbols,"$1 coeff_abs_level_greater1=%d\n",bit);

  return bit;
}