void read_mvd_coding(thread_context* tctx,
                     int x0,int y0, int refList)
{
  int abs_mvd_greater0_flag[2];
  abs_mvd_greater0_flag[0] = decode_CABAC_bit(&tctx->cabac_decoder,
                                              &tctx->ctx_model[CONTEXT_MODEL_ABS_MVD_GREATER01_FLAG+0]);
  abs_mvd_greater0_flag[1] = decode_CABAC_bit(&tctx->cabac_decoder,
                                              &tctx->ctx_model[CONTEXT_MODEL_ABS_MVD_GREATER01_FLAG+0]);

  int abs_mvd_greater1_flag[2];
  if (abs_mvd_greater0_flag[0]) {
    abs_mvd_greater1_flag[0] = decode_CABAC_bit(&tctx->cabac_decoder,
                                                &tctx->ctx_model[CONTEXT_MODEL_ABS_MVD_GREATER01_FLAG+1]);
  }
  else {
    abs_mvd_greater1_flag[0]=0;
  }

  if (abs_mvd_greater0_flag[1]) {
    abs_mvd_greater1_flag[1] = decode_CABAC_bit(&tctx->cabac_decoder,
                                                &tctx->ctx_model[CONTEXT_MODEL_ABS_MVD_GREATER01_FLAG+1]);
  }
  else {
    abs_mvd_greater1_flag[1]=0;
  }


  int abs_mvd_minus2[2];
  int mvd_sign_flag[2];
  int value[2];

  for (int c=0;c<2;c++) {
    if (abs_mvd_greater0_flag[c]) {
      if (abs_mvd_greater1_flag[c]) {
        abs_mvd_minus2[c] = decode_CABAC_EGk_bypass(&tctx->cabac_decoder, 1);
      }
      else {
        abs_mvd_minus2[c] = abs_mvd_greater1_flag[c] -1;
      }

      mvd_sign_flag[c] = decode_CABAC_bypass(&tctx->cabac_decoder);

      value[c] = abs_mvd_minus2[c]+2;
      if (mvd_sign_flag[c]) { value[c] = -value[c]; }
    }
    else {
      value[c] = 0;
    }
  }

  //set_mvd(tctx->decctx, x0,y0, refList, value[0],value[1]);
  tctx->motion.mvd[refList][0] = value[0];
  tctx->motion.mvd[refList][1] = value[1];

  logtrace(LogSlice, "MVD[%d;%d|%d] = %d;%d\n",x0,y0,refList, value[0],value[1]);
}