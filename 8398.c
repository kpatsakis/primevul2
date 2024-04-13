int residual_coding(thread_context* tctx,
                    int x0, int y0,  // position of TU in frame
                    int log2TrafoSize,
                    int cIdx)
{
  logtrace(LogSlice,"- residual_coding x0:%d y0:%d log2TrafoSize:%d cIdx:%d\n",x0,y0,log2TrafoSize,cIdx);

  //slice_segment_header* shdr = tctx->shdr;

  de265_image* img = tctx->img;
  const seq_parameter_set& sps = img->get_sps();
  const pic_parameter_set& pps = img->get_pps();

  enum PredMode PredMode = img->get_pred_mode(x0,y0);

  if (cIdx==0) {
    img->set_nonzero_coefficient(x0,y0,log2TrafoSize);
  }


  if (pps.transform_skip_enabled_flag &&
      !tctx->cu_transquant_bypass_flag &&
      (log2TrafoSize <= pps.Log2MaxTransformSkipSize))
    {
      tctx->transform_skip_flag[cIdx] = decode_transform_skip_flag(tctx,cIdx);
    }
  else
    {
      tctx->transform_skip_flag[cIdx] = 0;
    }


  tctx->explicit_rdpcm_flag = false;

  if (PredMode == MODE_INTER && sps.range_extension.explicit_rdpcm_enabled_flag &&
      ( tctx->transform_skip_flag[cIdx] || tctx->cu_transquant_bypass_flag))
    {
      tctx->explicit_rdpcm_flag = decode_explicit_rdpcm_flag(tctx,cIdx);
      if (tctx->explicit_rdpcm_flag) {
        tctx->explicit_rdpcm_dir = decode_explicit_rdpcm_dir(tctx,cIdx);
      }

      //printf("EXPLICIT RDPCM %d;%d\n",x0,y0);
    }
  else
    {
      tctx->explicit_rdpcm_flag = false;
    }



  // sbType for persistent_rice_adaptation_enabled_flag

  int sbType = (cIdx==0) ? 2 : 0;
  if (tctx->transform_skip_flag[cIdx] || tctx->cu_transquant_bypass_flag) {
    sbType++;
  }


  // --- decode position of last coded coefficient ---

  int last_significant_coeff_x_prefix =
    decode_last_significant_coeff_prefix(tctx,log2TrafoSize,cIdx,
                                         &tctx->ctx_model[CONTEXT_MODEL_LAST_SIGNIFICANT_COEFFICIENT_X_PREFIX]);

  int last_significant_coeff_y_prefix =
    decode_last_significant_coeff_prefix(tctx,log2TrafoSize,cIdx,
                                         &tctx->ctx_model[CONTEXT_MODEL_LAST_SIGNIFICANT_COEFFICIENT_Y_PREFIX]);


  // TODO: we can combine both FL-bypass calls into one, but the gain may be limited...

  int LastSignificantCoeffX;
  if (last_significant_coeff_x_prefix > 3) {
    int nBits = (last_significant_coeff_x_prefix>>1)-1;
    int last_significant_coeff_x_suffix = decode_CABAC_FL_bypass(&tctx->cabac_decoder,nBits);

    LastSignificantCoeffX =
      ((2+(last_significant_coeff_x_prefix & 1)) << nBits) + last_significant_coeff_x_suffix;
  }
  else {
    LastSignificantCoeffX = last_significant_coeff_x_prefix;
  }

  int LastSignificantCoeffY;
  if (last_significant_coeff_y_prefix > 3) {
    int nBits = (last_significant_coeff_y_prefix>>1)-1;
    int last_significant_coeff_y_suffix = decode_CABAC_FL_bypass(&tctx->cabac_decoder,nBits);

    LastSignificantCoeffY =
      ((2+(last_significant_coeff_y_prefix & 1)) << nBits) + last_significant_coeff_y_suffix;
  }
  else {
    LastSignificantCoeffY = last_significant_coeff_y_prefix;
  }



  // --- determine scanIdx ---

  int scanIdx;

  if (PredMode == MODE_INTRA) {
    if (cIdx==0) {
      scanIdx = get_intra_scan_idx(log2TrafoSize, img->get_IntraPredMode(x0,y0),  cIdx, &sps);
      //printf("luma scan idx=%d <- intra mode=%d\n",scanIdx, img->get_IntraPredMode(x0,y0));
    }
    else {
      scanIdx = get_intra_scan_idx(log2TrafoSize, img->get_IntraPredModeC(x0,y0), cIdx, &sps);
      //printf("chroma scan idx=%d <- intra mode=%d chroma:%d trsize:%d\n",scanIdx,
      //       img->get_IntraPredModeC(x0,y0), sps->chroma_format_idc, 1<<log2TrafoSize);
    }
  }
  else {
    scanIdx=0;
  }

  if (scanIdx==2) {
    std::swap(LastSignificantCoeffX, LastSignificantCoeffY);
  }

  logtrace(LogSlice,"LastSignificantCoeff: x=%d;y=%d\n",LastSignificantCoeffX,LastSignificantCoeffY);

  const position* ScanOrderSub = get_scan_order(log2TrafoSize-2, scanIdx);
  const position* ScanOrderPos = get_scan_order(2, scanIdx);

  logtrace(LogSlice,"ScanOrderPos: ");
  for (int n=0;n<4*4;n++)
    logtrace(LogSlice,"*%d,%d ", ScanOrderPos[n].x, ScanOrderPos[n].y);
  logtrace(LogSlice,"*\n");


  // --- find last sub block and last scan pos ---

  int xC,yC;

  scan_position lastScanP = get_scan_position(LastSignificantCoeffX, LastSignificantCoeffY,
                                              scanIdx, log2TrafoSize);

  int lastScanPos  = lastScanP.scanPos;
  int lastSubBlock = lastScanP.subBlock;


  int sbWidth = 1<<(log2TrafoSize-2);

  uint8_t coded_sub_block_neighbors[32/4*32/4];
  memset(coded_sub_block_neighbors,0,sbWidth*sbWidth);

  int  c1 = 1;
  bool firstSubblock = true;           // for coeff_abs_level_greater1_flag context model
  int  lastSubblock_greater1Ctx=false; /* for coeff_abs_level_greater1_flag context model
                                          (initialization not strictly needed)
                                       */

#ifdef DE265_LOG_TRACE
  int16_t TransCoeffLevel[32 * 32];
  memset(TransCoeffLevel,0, sizeof(uint16_t)*32*32);
#endif

  int CoeffStride = 1<<log2TrafoSize;

  int  lastInvocation_greater1Ctx=0;
  int  lastInvocation_coeff_abs_level_greater1_flag=0;
  int  lastInvocation_ctxSet=0;



  // ----- decode coefficients -----

  tctx->nCoeff[cIdx] = 0;


  // i - subblock index
  // n - coefficient index in subblock

  for (int i=lastSubBlock;i>=0;i--) {
    position S = ScanOrderSub[i];
    int inferSbDcSigCoeffFlag=0;

    logtrace(LogSlice,"sub block scan idx: %d\n",i);


    // --- check whether this sub-block is coded ---

    int sub_block_is_coded = 0;

    if ((i<lastSubBlock) && (i>0)) {
      sub_block_is_coded = decode_coded_sub_block_flag(tctx, cIdx,
                                                       coded_sub_block_neighbors[S.x+S.y*sbWidth]);
      inferSbDcSigCoeffFlag=1;
    }
    else if (i==0 || i==lastSubBlock) {
      // first (DC) and last sub-block are always coded
      // - the first will most probably contain coefficients
      // - the last obviously contains the last coded coefficient

      sub_block_is_coded = 1;
    }

    if (sub_block_is_coded) {
      if (S.x > 0) coded_sub_block_neighbors[S.x-1 + S.y  *sbWidth] |= 1;
      if (S.y > 0) coded_sub_block_neighbors[S.x + (S.y-1)*sbWidth] |= 2;
    }


    // ----- find significant coefficients in this sub-block -----

    int16_t  coeff_value[16];
    int8_t   coeff_scan_pos[16];
    int8_t   coeff_sign[16];
    int8_t   coeff_has_max_base_level[16];
    int nCoefficients=0;


    if (sub_block_is_coded) {
      int x0 = S.x<<2;
      int y0 = S.y<<2;

      int log2w = log2TrafoSize-2;
      int prevCsbf = coded_sub_block_neighbors[S.x+S.y*sbWidth];
      uint8_t* ctxIdxMap = ctxIdxLookup[log2w][!!cIdx][!!scanIdx][prevCsbf];

      logdebug(LogSlice,"log2w:%d cIdx:%d scanIdx:%d prevCsbf:%d\n",
               log2w,cIdx,scanIdx,prevCsbf);


      // set the last coded coefficient in the last subblock

      int last_coeff =  (i==lastSubBlock) ? lastScanPos-1 : 15;

      if (i==lastSubBlock) {
        coeff_value[nCoefficients] = 1;
        coeff_has_max_base_level[nCoefficients] = 1;
        coeff_scan_pos[nCoefficients] = lastScanPos;
        nCoefficients++;
      }


      // --- decode all coefficients' significant_coeff flags except for the DC coefficient ---

      for (int n= last_coeff ; n>0 ; n--) {
        int subX = ScanOrderPos[n].x;
        int subY = ScanOrderPos[n].y;
        xC = x0 + subX;
        yC = y0 + subY;


        // for all AC coefficients in sub-block, a significant_coeff flag is coded

        int ctxInc;
        if (sps.range_extension.transform_skip_context_enabled_flag &&
            (tctx->cu_transquant_bypass_flag || tctx->transform_skip_flag[cIdx])) {
          ctxInc = ( cIdx == 0 ) ? 42 : (16+27);
        }
        else {
          ctxInc = ctxIdxMap[xC+(yC<<log2TrafoSize)];
        }

        logtrace(LogSlice,"trafoSize: %d\n",1<<log2TrafoSize);

        int significant_coeff = decode_significant_coeff_flag_lookup(tctx, ctxInc);

        if (significant_coeff) {
          coeff_value[nCoefficients] = 1;
          coeff_has_max_base_level[nCoefficients] = 1;
          coeff_scan_pos[nCoefficients] = n;
          nCoefficients++;

          // since we have a coefficient in the sub-block,
          // we cannot infer the DC coefficient anymore
          inferSbDcSigCoeffFlag = 0;
        }
      }


      // --- decode DC coefficient significance ---

      if (last_coeff>=0) // last coded coefficient (always set to 1) is not the DC coefficient
        {
          if (inferSbDcSigCoeffFlag==0) {
            // if we cannot infert the DC coefficient, it is coded

            int ctxInc;
            if (sps.range_extension.transform_skip_context_enabled_flag &&
                (tctx->cu_transquant_bypass_flag || tctx->transform_skip_flag[cIdx])) {
              ctxInc = ( cIdx == 0 ) ? 42 : (16+27);
            }
            else {
              ctxInc = ctxIdxMap[x0+(y0<<log2TrafoSize)];
            }

            int significant_coeff = decode_significant_coeff_flag_lookup(tctx, ctxInc);


            if (significant_coeff) {
              coeff_value[nCoefficients] = 1;
              coeff_has_max_base_level[nCoefficients] = 1;
              coeff_scan_pos[nCoefficients] = 0;
              nCoefficients++;
            }
          }
          else {
            // we can infer that the DC coefficient must be present
            coeff_value[nCoefficients] = 1;
            coeff_has_max_base_level[nCoefficients] = 1;
            coeff_scan_pos[nCoefficients] = 0;
            nCoefficients++;
          }
        }

    }


    /*
      logtrace(LogSlice,"significant_coeff_flags:\n");
      for (int y=0;y<4;y++) {
      logtrace(LogSlice,"  ");
      for (int x=0;x<4;x++) {
      logtrace(LogSlice,"*%d ",significant_coeff_flag[y][x]);
      }
      logtrace(LogSlice,"*\n");
      }
    */


    if (nCoefficients) {
      int ctxSet;
      if (i==0 || cIdx>0) { ctxSet=0; }
      else { ctxSet=2; }

      if (c1==0) { ctxSet++; }
      c1=1;


      // --- decode greater-1 flags ---

      int newLastGreater1ScanPos=-1;

      int lastGreater1Coefficient = libde265_min(8,nCoefficients);
      for (int c=0;c<lastGreater1Coefficient;c++) {
        int greater1_flag =
          decode_coeff_abs_level_greater1(tctx, cIdx,i,
                                          c==0,
                                          firstSubblock,
                                          lastSubblock_greater1Ctx,
                                          &lastInvocation_greater1Ctx,
                                          &lastInvocation_coeff_abs_level_greater1_flag,
                                          &lastInvocation_ctxSet, ctxSet);

        if (greater1_flag) {
          coeff_value[c]++;

          c1=0;

          if (newLastGreater1ScanPos == -1) {
            newLastGreater1ScanPos=c;
          }
        }
        else {
          coeff_has_max_base_level[c] = 0;

          if (c1<3 && c1>0) {
            c1++;
          }
        }
      }

      firstSubblock = false;
      lastSubblock_greater1Ctx = lastInvocation_greater1Ctx;


      // --- decode greater-2 flag ---

      if (newLastGreater1ScanPos != -1) {
        int flag = decode_coeff_abs_level_greater2(tctx,cIdx, lastInvocation_ctxSet);
        coeff_value[newLastGreater1ScanPos] += flag;
        coeff_has_max_base_level[newLastGreater1ScanPos] = flag;
      }


      // --- decode coefficient signs ---

      int signHidden;


      IntraPredMode predModeIntra;
      if (cIdx==0) predModeIntra = img->get_IntraPredMode(x0,y0);
      else         predModeIntra = img->get_IntraPredModeC(x0,y0);


      if (tctx->cu_transquant_bypass_flag ||
          (PredMode == MODE_INTRA &&
           sps.range_extension.implicit_rdpcm_enabled_flag &&
           tctx->transform_skip_flag[cIdx] &&
           ( predModeIntra == 10 || predModeIntra == 26 )) ||
          tctx->explicit_rdpcm_flag)
        {
          signHidden = 0;
        }
      else
        {
          signHidden = (coeff_scan_pos[0]-coeff_scan_pos[nCoefficients-1] > 3);
        }


      for (int n=0;n<nCoefficients-1;n++) {
        coeff_sign[n] = decode_CABAC_bypass(&tctx->cabac_decoder);
        logtrace(LogSlice,"sign[%d] = %d\n", n, coeff_sign[n]);
      }

      // n==nCoefficients-1
      if (!pps.sign_data_hiding_flag || !signHidden) {
        coeff_sign[nCoefficients-1] = decode_CABAC_bypass(&tctx->cabac_decoder);
        logtrace(LogSlice,"sign[%d] = %d\n", nCoefficients-1, coeff_sign[nCoefficients-1]);
      }
      else {
        coeff_sign[nCoefficients-1] = 0;
      }


      // --- decode coefficient value ---

      int sumAbsLevel=0;
      int uiGoRiceParam;

      if (sps.range_extension.persistent_rice_adaptation_enabled_flag==0) {
        uiGoRiceParam = 0;
      }
      else {
        uiGoRiceParam = tctx->StatCoeff[sbType]/4;
      }

      // printf("initial uiGoRiceParam=%d\n",uiGoRiceParam);
      bool firstCoeffWithAbsLevelRemaining = true;

      for (int n=0;n<nCoefficients;n++) {
        int baseLevel = coeff_value[n];

        int coeff_abs_level_remaining;

        // printf("coeff %d/%d, uiRiceParam: %d\n",n,nCoefficients,uiGoRiceParam);

        if (coeff_has_max_base_level[n]) {
          coeff_abs_level_remaining =
            decode_coeff_abs_level_remaining(tctx, uiGoRiceParam);

          if (sps.range_extension.persistent_rice_adaptation_enabled_flag == 0) {
            // (2014.10 / 9-20)
            if (baseLevel + coeff_abs_level_remaining > 3*(1<<uiGoRiceParam)) {
              uiGoRiceParam++;
              if (uiGoRiceParam>4) uiGoRiceParam=4;
            }
          }
          else {
            if (baseLevel + coeff_abs_level_remaining > 3*(1<<uiGoRiceParam))
              uiGoRiceParam++;
          }

          // persistent_rice_adaptation_enabled_flag
          if (sps.range_extension.persistent_rice_adaptation_enabled_flag &&
              firstCoeffWithAbsLevelRemaining) {
            if (coeff_abs_level_remaining >= (3 << (tctx->StatCoeff[sbType]/4 ))) {
              tctx->StatCoeff[sbType]++;
            }
            else if (2*coeff_abs_level_remaining < (1 << (tctx->StatCoeff[sbType]/4 )) &&
                     tctx->StatCoeff[sbType] > 0) {
              tctx->StatCoeff[sbType]--;
            }
          }

          firstCoeffWithAbsLevelRemaining=false;
        }
        else {
          coeff_abs_level_remaining = 0;
        }

        logtrace(LogSlice, "coeff_abs_level_remaining=%d\n",coeff_abs_level_remaining);


        int16_t currCoeff = baseLevel + coeff_abs_level_remaining;
        if (coeff_sign[n]) {
          currCoeff = -currCoeff;
        }

        if (pps.sign_data_hiding_flag && signHidden) {
          sumAbsLevel += baseLevel + coeff_abs_level_remaining;

          if (n==nCoefficients-1 && (sumAbsLevel & 1)) {
            currCoeff = -currCoeff;
          }
        }

        logtrace(LogSlice, "quantized coefficient=%d\n",currCoeff);

#ifdef DE265_LOG_TRACE
        //TransCoeffLevel[yC*CoeffStride + xC] = currCoeff;
#endif

        // put coefficient in list
        int p = coeff_scan_pos[n];
        xC = (S.x<<2) + ScanOrderPos[p].x;
        yC = (S.y<<2) + ScanOrderPos[p].y;

        tctx->coeffList[cIdx][ tctx->nCoeff[cIdx] ] = currCoeff;
        tctx->coeffPos [cIdx][ tctx->nCoeff[cIdx] ] = xC + yC*CoeffStride;
        tctx->nCoeff[cIdx]++;

        //printf("%d ",currCoeff);
      }  // iterate through coefficients in sub-block

      //printf(" (%d;%d)\n",x0,y0);

    }  // if nonZero
  }  // next sub-block

  return DE265_OK;
}