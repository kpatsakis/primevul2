static int decode_significant_coeff_flag(thread_context* tctx,
					 int xC,int yC,
					 const uint8_t* coded_sub_block_flag,
					 int sbWidth,
					 int cIdx,
					 int scanIdx)
{
  logtrace(LogSlice,"# significant_coeff_flag (xC:%d yC:%d sbWidth:%d cIdx:%d scanIdx:%d)\n",
           xC,yC,sbWidth,cIdx,scanIdx);

  int sigCtx;

  // if log2TrafoSize==2
  if (sbWidth==1) {
    sigCtx = ctxIdxMap[(yC<<2) + xC];
  }
  else if (xC+yC==0) {
    sigCtx = 0;
  }
  else {
    int xS = xC>>2;
    int yS = yC>>2;
    int prevCsbf = 0;
    if (xS < sbWidth-1) { prevCsbf += coded_sub_block_flag[xS+1  +yS*sbWidth];    }
    if (yS < sbWidth-1) { prevCsbf += coded_sub_block_flag[xS+(1+yS)*sbWidth]<<1; }

    int xP = xC & 3;
    int yP = yC & 3;

    logtrace(LogSlice,"posInSubset: %d,%d\n",xP,yP);
    logtrace(LogSlice,"prevCsbf: %d\n",prevCsbf);

    //printf("%d | %d %d\n",prevCsbf,xP,yP);

    switch (prevCsbf) {
    case 0:
      //sigCtx = (xP+yP==0) ? 2 : (xP+yP<3) ? 1 : 0;
      sigCtx = (xP+yP>=3) ? 0 : (xP+yP>0) ? 1 : 2;
      break;
    case 1:
      sigCtx = (yP==0) ? 2 : (yP==1) ? 1 : 0;
      break;
    case 2:
      sigCtx = (xP==0) ? 2 : (xP==1) ? 1 : 0;
      break;
    default:
      sigCtx = 2;
      break;
    }

    logtrace(LogSlice,"a) sigCtx=%d\n",sigCtx);

    if (cIdx==0) {
      if (xS+yS > 0) sigCtx+=3;

      logtrace(LogSlice,"b) sigCtx=%d\n",sigCtx);

      // if log2TrafoSize==3
      if (sbWidth==2) {
        sigCtx += (scanIdx==0) ? 9 : 15;
      } else {
        sigCtx += 21;
      }

      logtrace(LogSlice,"c) sigCtx=%d\n",sigCtx);
    }
    else {
      // if log2TrafoSize==3
      if (sbWidth==2) {
        sigCtx+=9;
      }
      else {
        sigCtx+=12;
      }
    }
  }

  int ctxIdxInc;
  if (cIdx==0) { ctxIdxInc=sigCtx; }
  else         { ctxIdxInc=27+sigCtx; }

  int context = tctx->shdr->initType*42 + ctxIdxInc;
  logtrace(LogSlice,"context: %d\n",context);

  int bit = decode_CABAC_bit(&tctx->cabac_decoder,
                             &tctx->ctx_model[CONTEXT_MODEL_SIGNIFICANT_COEFF_FLAG + context]);
  return bit;
}