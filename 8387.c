bool alloc_and_init_significant_coeff_ctxIdx_lookupTable_OLD()
{
  int tableSize = 2*2*4*(4*4 + 8*8 + 16*16 + 32*32);
  uint8_t* p = (uint8_t*)malloc(tableSize);
  if (p==NULL) {
    return false;
  }

  for (int log2w=2; log2w<=5 ; log2w++)
    for (int cIdx=0;cIdx<2;cIdx++)
      for (int scanIdx=0;scanIdx<2;scanIdx++)
        for (int prevCsbf=0;prevCsbf<4;prevCsbf++)
          {
            // assign pointer into reserved memory area

            ctxIdxLookup[log2w-2][cIdx][scanIdx][prevCsbf] = p;
            p += (1<<log2w)*(1<<log2w);

            const position* ScanOrderSub = get_scan_order(log2w-2, scanIdx);
            const position* ScanOrderPos = get_scan_order(2, scanIdx);

            //for (int yC=0;yC<(1<<log2w);yC++)
            // for (int xC=0;xC<(1<<log2w);xC++)
            for (int s=0;s<(1<<log2w)*(1<<log2w);s++)
              {
                position S = ScanOrderSub[s>>4];
                int x0 = S.x<<2;
                int y0 = S.y<<2;

                int subX = ScanOrderPos[s & 0xF].x;
                int subY = ScanOrderPos[s & 0xF].y;
                int xC = x0 + subX;
                int yC = y0 + subY;


                int w = 1<<log2w;
                int sbWidth = w>>2;

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
                  /*
                    int prevCsbf = 0;

                    if (xS < sbWidth-1) { prevCsbf += coded_sub_block_flag[xS+1  +yS*sbWidth];    }
                    if (yS < sbWidth-1) { prevCsbf += coded_sub_block_flag[xS+(1+yS)*sbWidth]<<1; }
                  */
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
                    if (sbWidth==2) { // 8x8 block
                      sigCtx += (scanIdx==0) ? 9 : 15;
                    } else {
                      sigCtx += 21;
                    }

                    logtrace(LogSlice,"c) sigCtx=%d\n",sigCtx);
                  }
                  else {
                    // if log2TrafoSize==3
                    if (sbWidth==2) { // 8x8 block
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


                ctxIdxLookup[log2w-2][cIdx][scanIdx][prevCsbf][xC+(yC<<log2w)] = ctxIdxInc;

                //NOTE: when using this option, we have to include all three scanIdx in the table
                //ctxIdxLookup[log2w-2][cIdx][scanIdx][prevCsbf][s] = ctxIdxInc;
              }
          }

  return true;
}