int read_transform_unit(thread_context* tctx,
                        int x0, int y0,        // position of TU in frame
                        int xBase, int yBase,  // position of parent TU in frame
                        int xCUBase,int yCUBase,  // position of CU in frame
                        int log2TrafoSize,
                        int trafoDepth,
                        int blkIdx,
                        int cbf_luma, int cbf_cb, int cbf_cr)
{
  logtrace(LogSlice,"- read_transform_unit x0:%d y0:%d xBase:%d yBase:%d nT:%d cbf:%d:%d:%d\n",
           x0,y0,xBase,yBase, 1<<log2TrafoSize, cbf_luma, cbf_cb, cbf_cr);

  assert(cbf_cb != -1);
  assert(cbf_cr != -1);
  assert(cbf_luma != -1);

  const seq_parameter_set& sps = tctx->img->get_sps();

  const int ChromaArrayType = sps.ChromaArrayType;

  int log2TrafoSizeC = (ChromaArrayType==CHROMA_444 ? log2TrafoSize : log2TrafoSize-1);
  log2TrafoSizeC = libde265_max(2, log2TrafoSizeC);

  const int cbfLuma   = cbf_luma;
  const int cbfChroma = cbf_cb | cbf_cr;

  tctx->transform_skip_flag[0]=0;
  tctx->transform_skip_flag[1]=0;
  tctx->transform_skip_flag[2]=0;

  tctx->explicit_rdpcm_flag = false;


  enum PredMode cuPredMode = tctx->img->get_pred_mode(x0,y0);

  if (cbfLuma || cbfChroma)
    {
      bool doDecodeQuantParameters = false;

      if (tctx->img->get_pps().cu_qp_delta_enabled_flag &&
          !tctx->IsCuQpDeltaCoded) {

        int cu_qp_delta_abs = decode_cu_qp_delta_abs(tctx);
        int cu_qp_delta_sign=0;
        if (cu_qp_delta_abs) {
          cu_qp_delta_sign = decode_CABAC_bypass(&tctx->cabac_decoder);
        }

        tctx->IsCuQpDeltaCoded = 1;
        tctx->CuQpDelta = cu_qp_delta_abs*(1-2*cu_qp_delta_sign);

        //printf("read cu_qp_delta (%d;%d) = %d\n",x0,y0,tctx->CuQpDelta);

        logtrace(LogSlice,"cu_qp_delta_abs = %d\n",cu_qp_delta_abs);
        logtrace(LogSlice,"cu_qp_delta_sign = %d\n",cu_qp_delta_sign);
        logtrace(LogSlice,"CuQpDelta = %d\n",tctx->CuQpDelta);

        doDecodeQuantParameters = true;
        //decode_quantization_parameters(tctx, x0,y0, xCUBase, yCUBase);
      }

      if (tctx->shdr->cu_chroma_qp_offset_enabled_flag && cbfChroma &&
          !tctx->cu_transquant_bypass_flag && !tctx->IsCuChromaQpOffsetCoded ) {
        logtrace(LogSlice,"# cu_chroma_qp_offset_flag\n");

        int cu_chroma_qp_offset_flag = decode_CABAC_bit(&tctx->cabac_decoder,
                                                        &tctx->ctx_model[CONTEXT_MODEL_CU_CHROMA_QP_OFFSET_FLAG]);


        const pic_parameter_set& pps = tctx->img->get_pps();

        int cu_chroma_qp_offset_idx = 0;
        if (cu_chroma_qp_offset_flag && pps.range_extension.chroma_qp_offset_list_len > 1) {
          cu_chroma_qp_offset_idx = decode_CABAC_bit(&tctx->cabac_decoder,
                                                     &tctx->ctx_model[CONTEXT_MODEL_CU_CHROMA_QP_OFFSET_IDX]);
        }

        tctx->IsCuChromaQpOffsetCoded = 1;

        if (cu_chroma_qp_offset_flag) {
          tctx->CuQpOffsetCb = pps.range_extension.cb_qp_offset_list[ cu_chroma_qp_offset_idx ];
          tctx->CuQpOffsetCr = pps.range_extension.cr_qp_offset_list[ cu_chroma_qp_offset_idx ];
        }
        else {
          tctx->CuQpOffsetCb = 0;
          tctx->CuQpOffsetCr = 0;
        }

        doDecodeQuantParameters = true;
        //decode_quantization_parameters(tctx, x0,y0, xCUBase, yCUBase);
      }


      if (doDecodeQuantParameters) {
        decode_quantization_parameters(tctx, x0,y0, xCUBase, yCUBase);
      }
    }

  // position of TU in local CU
  int xL = x0 - xCUBase;
  int yL = y0 - yCUBase;
  int nT = 1<<log2TrafoSize;
  int nTC = 1<<log2TrafoSizeC;

  const int SubWidthC  = sps.SubWidthC;
  const int SubHeightC = sps.SubHeightC;

  // --- luma ---

  tctx->ResScaleVal = 0;

  int err;
  if (cbf_luma) {
    if ((err=residual_coding(tctx,x0,y0, log2TrafoSize,0)) != DE265_OK) return err;
  }

  decode_TU(tctx, x0,y0, xCUBase,yCUBase, nT, 0, cuPredMode, cbf_luma);


  // --- chroma ---

  const int yOffset422 = 1<<log2TrafoSizeC;

  if (log2TrafoSize>2 || ChromaArrayType == CHROMA_444) {
    // TODO: cross-component prediction

    const bool do_cross_component_prediction =
      (tctx->img->get_pps().range_extension.cross_component_prediction_enabled_flag &&
       cbf_luma &&
       (cuPredMode == MODE_INTER || tctx->img->is_IntraPredModeC_Mode4(x0,y0)));

    if (do_cross_component_prediction) {
      read_cross_comp_pred(tctx, 0);
    }
    else {
      tctx->ResScaleVal = 0;
    }

    {
      if (cbf_cb & 1) {
        if ((err=residual_coding(tctx,x0,y0,log2TrafoSizeC,1)) != DE265_OK) return err;
      }

      if (sps.ChromaArrayType != CHROMA_MONO) {
        decode_TU(tctx,
                  x0/SubWidthC,y0/SubHeightC,
                  xCUBase/SubWidthC,yCUBase/SubHeightC, nTC, 1, cuPredMode, cbf_cb & 1);
      }
    }

    // 4:2:2
    if (ChromaArrayType == CHROMA_422) {
      const int yOffset = 1<<log2TrafoSizeC;

      if (cbf_cb & 2) {
        if ((err=residual_coding(tctx,
                                 x0,y0+yOffset*SubHeightC,
                                 log2TrafoSizeC,1)) != DE265_OK) return err;
      }

      decode_TU(tctx,
                x0/SubWidthC,y0/SubHeightC + yOffset,
                xCUBase/SubWidthC,yCUBase/SubHeightC +yOffset,
                nTC, 1, cuPredMode, cbf_cb & 2);
    }


    if (do_cross_component_prediction) {
      read_cross_comp_pred(tctx, 1);
    }
    else {
      tctx->ResScaleVal = 0;
    }

    {
      if (cbf_cr & 1) {
        if ((err=residual_coding(tctx,x0,y0,log2TrafoSizeC,2)) != DE265_OK) return err;
      }

      if (sps.ChromaArrayType != CHROMA_MONO) {
        decode_TU(tctx,
                  x0/SubWidthC,y0/SubHeightC,
                  xCUBase/SubWidthC,yCUBase/SubHeightC,
                  nTC, 2, cuPredMode, cbf_cr & 1);
      }
    }

    // 4:2:2
    if (ChromaArrayType == CHROMA_422) {
      const int yOffset = 1<<log2TrafoSizeC;

      if (cbf_cr & 2) {
        if ((err=residual_coding(tctx,
                                 x0,y0+yOffset*SubHeightC,
                                 log2TrafoSizeC,2)) != DE265_OK) return err;
      }

      decode_TU(tctx,
                x0/SubWidthC,y0/SubHeightC+yOffset,
                xCUBase/SubWidthC,yCUBase/SubHeightC+yOffset,
                nTC, 2, cuPredMode, cbf_cr & 2);
    }
  }
  else if (blkIdx==3) {
    if (cbf_cb & 1) {
      if ((err=residual_coding(tctx,xBase,yBase,
                               log2TrafoSize,1)) != DE265_OK) return err;
    }

    if (sps.ChromaArrayType != CHROMA_MONO) {
      decode_TU(tctx,
                xBase/SubWidthC,  yBase/SubHeightC,
                xCUBase/SubWidthC,yCUBase/SubHeightC, nT, 1, cuPredMode, cbf_cb & 1);
    }

    // 4:2:2
    if (cbf_cb & 2) {
      if ((err=residual_coding(tctx,
                               xBase        ,yBase        +(1<<log2TrafoSize),
                               log2TrafoSize,1)) != DE265_OK) return err;
    }

    if (ChromaArrayType == CHROMA_422) {
      decode_TU(tctx,
                xBase/SubWidthC,  yBase/SubHeightC + (1<<log2TrafoSize),
                xCUBase/SubWidthC,yCUBase/SubHeightC, nT, 1, cuPredMode, cbf_cb & 2);
    }

    if (cbf_cr & 1) {
      if ((err=residual_coding(tctx,xBase,yBase,
                               log2TrafoSize,2)) != DE265_OK) return err;
    }

    if (sps.ChromaArrayType != CHROMA_MONO) {
      decode_TU(tctx,
                xBase/SubWidthC,  yBase/SubHeightC,
                xCUBase/SubWidthC,yCUBase/SubHeightC, nT, 2, cuPredMode, cbf_cr & 1);
    }

    // 4:2:2
    if (cbf_cr & 2) {
      if ((err=residual_coding(tctx,
                               xBase        ,yBase        +(1<<log2TrafoSizeC),
                               log2TrafoSize,2)) != DE265_OK) return err;
    }

    if (ChromaArrayType == CHROMA_422) {
      decode_TU(tctx,
                xBase/SubWidthC,  yBase/SubHeightC + (1<<log2TrafoSize),
                xCUBase/SubWidthC,yCUBase/SubHeightC, nT, 2, cuPredMode, cbf_cr & 2);
    }
  }


  return DE265_OK;
}