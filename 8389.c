static void decode_TU(thread_context* tctx,
                      int x0,int y0,
                      int xCUBase,int yCUBase,
                      int nT, int cIdx, enum PredMode cuPredMode, bool cbf)
{
  de265_image* img = tctx->img;
  const seq_parameter_set& sps = img->get_sps();

  int residualDpcm = 0;

  if (cuPredMode == MODE_INTRA) // if intra mode
    {
      enum IntraPredMode intraPredMode;

      if (cIdx==0) {
        intraPredMode = img->get_IntraPredMode(x0,y0);
      }
      else {
        const int SubWidthC  = sps.SubWidthC;
        const int SubHeightC = sps.SubHeightC;

        intraPredMode = img->get_IntraPredModeC(x0*SubWidthC,y0*SubHeightC);
      }

      if (intraPredMode<0 || intraPredMode>=35) {
        // TODO: ERROR
        intraPredMode = INTRA_DC;
      }

      decode_intra_prediction(img, x0,y0, intraPredMode, nT, cIdx);


      residualDpcm = sps.range_extension.implicit_rdpcm_enabled_flag &&
        (tctx->cu_transquant_bypass_flag || tctx->transform_skip_flag[cIdx]) &&
        (intraPredMode == 10 || intraPredMode == 26);

      if (residualDpcm && intraPredMode == 26)
        residualDpcm = 2;
    }
  else // INTER
    {
      if (tctx->explicit_rdpcm_flag) {
        residualDpcm = (tctx->explicit_rdpcm_dir ? 2 : 1);
      }
    }

  if (cbf) {
    scale_coefficients(tctx, x0,y0, xCUBase,yCUBase, nT, cIdx,
                       tctx->transform_skip_flag[cIdx], cuPredMode==MODE_INTRA, residualDpcm);
  }
  /*
  else if (!cbf && cIdx==0) {
    memset(tctx->residual_luma,0,32*32*sizeof(int32_t));
  }
  */
  else if (!cbf && cIdx!=0 && tctx->ResScaleVal) {
    // --- cross-component-prediction when CBF==0 ---

    tctx->nCoeff[cIdx] = 0;
    residualDpcm=0;

    scale_coefficients(tctx, x0,y0, xCUBase,yCUBase, nT, cIdx,
                       tctx->transform_skip_flag[cIdx], cuPredMode==MODE_INTRA, residualDpcm);
  }
}