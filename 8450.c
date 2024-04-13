bool alloc_and_init_significant_coeff_ctxIdx_lookupTable()
{
  int tableSize = 4*4*(2) + 8*8*(2*2*4) + 16*16*(2*4) + 32*32*(2*4);

  uint8_t* p = (uint8_t*)malloc(tableSize);
  if (p==NULL) {
    return false;
  }

  memset(p,0xFF,tableSize);  // just for debugging


  // --- Set pointers to memory areas. Note that some parameters share the same memory. ---

  // 4x4

  for (int cIdx=0;cIdx<2;cIdx++) {
    for (int scanIdx=0;scanIdx<2;scanIdx++)
      for (int prevCsbf=0;prevCsbf<4;prevCsbf++)
        ctxIdxLookup[0][cIdx][scanIdx][prevCsbf] = p;

    p += 4*4;
  }

  // 8x8

  for (int cIdx=0;cIdx<2;cIdx++)
    for (int scanIdx=0;scanIdx<2;scanIdx++)
      for (int prevCsbf=0;prevCsbf<4;prevCsbf++) {
        ctxIdxLookup[1][cIdx][scanIdx][prevCsbf] = p;
        p += 8*8;
      }

  // 16x16

  for (int cIdx=0;cIdx<2;cIdx++)
    for (int prevCsbf=0;prevCsbf<4;prevCsbf++) {
      for (int scanIdx=0;scanIdx<2;scanIdx++) {
        ctxIdxLookup[2][cIdx][scanIdx][prevCsbf] = p;
      }

      p += 16*16;
    }

  // 32x32

  for (int cIdx=0;cIdx<2;cIdx++)
    for (int prevCsbf=0;prevCsbf<4;prevCsbf++) {
      for (int scanIdx=0;scanIdx<2;scanIdx++) {
        ctxIdxLookup[3][cIdx][scanIdx][prevCsbf] = p;
      }

      p += 32*32;
    }


  // --- precompute ctxIdx tables ---

  for (int log2w=2; log2w<=5 ; log2w++)
    for (int cIdx=0;cIdx<2;cIdx++)
      for (int scanIdx=0;scanIdx<2;scanIdx++)
        for (int prevCsbf=0;prevCsbf<4;prevCsbf++)
          {
            for (int yC=0;yC<(1<<log2w);yC++)
              for (int xC=0;xC<(1<<log2w);xC++)
                {
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

                    //logtrace(LogSlice,"posInSubset: %d,%d\n",xP,yP);
                    //logtrace(LogSlice,"prevCsbf: %d\n",prevCsbf);

                    switch (prevCsbf) {
                    case 0:
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

                    //logtrace(LogSlice,"a) sigCtx=%d\n",sigCtx);

                    if (cIdx==0) {
                      if (xS+yS > 0) sigCtx+=3;

                      //logtrace(LogSlice,"b) sigCtx=%d\n",sigCtx);

                      // if log2TrafoSize==3
                      if (sbWidth==2) { // 8x8 block
                        sigCtx += (scanIdx==0) ? 9 : 15;
                      } else {
                        sigCtx += 21;
                      }

                      //logtrace(LogSlice,"c) sigCtx=%d\n",sigCtx);
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

                  if (ctxIdxLookup[log2w-2][cIdx][scanIdx][prevCsbf][xC+(yC<<log2w)] != 0xFF) {
                    assert(ctxIdxLookup[log2w-2][cIdx][scanIdx][prevCsbf][xC+(yC<<log2w)] == ctxIdxInc);
                  }

                  ctxIdxLookup[log2w-2][cIdx][scanIdx][prevCsbf][xC+(yC<<log2w)] = ctxIdxInc;

                  //NOTE: when using this option, we have to include all three scanIdx in the table
                  //ctxIdxLookup[log2w-2][cIdx][scanIdx][prevCsbf][s] = ctxIdxInc;
                }
          }

  return true;
}