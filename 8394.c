void read_prediction_unit(thread_context* tctx,
                          int xC,int yC, int xB,int yB,
                          int nPbW, int nPbH,
                          int ctDepth, int nCS,int partIdx)
{
  logtrace(LogSlice,"read_prediction_unit %d;%d %dx%d\n",xC+xB,yC+xB,nPbW,nPbH);

  int x0 = xC+xB;
  int y0 = yC+yB;

  slice_segment_header* shdr = tctx->shdr;

  int merge_flag = decode_merge_flag(tctx);
  tctx->motion.merge_flag = merge_flag;

  if (merge_flag) {
    int merge_idx = decode_merge_idx(tctx);

    logtrace(LogSlice,"prediction unit %d,%d, merge mode, index: %d\n",x0,y0,merge_idx);

    tctx->motion.merge_idx = merge_idx;
  }
  else { // no merge flag
    enum InterPredIdc inter_pred_idc;

    if (shdr->slice_type == SLICE_TYPE_B) {
      inter_pred_idc = decode_inter_pred_idc(tctx,x0,y0,nPbW,nPbH,ctDepth);
    }
    else {
      inter_pred_idc = PRED_L0;
    }

    tctx->motion.inter_pred_idc = inter_pred_idc; // set_inter_pred_idc(ctx,x0,y0, inter_pred_idc);

    if (inter_pred_idc != PRED_L1) {
      int ref_idx_l0 = decode_ref_idx_lX(tctx, shdr->num_ref_idx_l0_active);

      // NOTE: case for only one reference frame is handles in decode_ref_idx_lX()
      tctx->motion.refIdx[0] = ref_idx_l0;

      read_mvd_coding(tctx,x0,y0, 0);

      int mvp_l0_flag = decode_mvp_lx_flag(tctx); // l0
      tctx->motion.mvp_l0_flag = mvp_l0_flag;

      logtrace(LogSlice,"prediction unit %d,%d, L0, refIdx=%d mvp_l0_flag:%d\n",
               x0,y0, tctx->motion.refIdx[0], mvp_l0_flag);
    }

    if (inter_pred_idc != PRED_L0) {
      int ref_idx_l1 = decode_ref_idx_lX(tctx, shdr->num_ref_idx_l1_active);

      // NOTE: case for only one reference frame is handles in decode_ref_idx_lX()
      tctx->motion.refIdx[1] = ref_idx_l1;

      if (shdr->mvd_l1_zero_flag &&
          inter_pred_idc == PRED_BI) {
        tctx->motion.mvd[1][0] = 0;
        tctx->motion.mvd[1][1] = 0;
      }
      else {
        read_mvd_coding(tctx,x0,y0, 1);
      }

      int mvp_l1_flag = decode_mvp_lx_flag(tctx); // l1
      tctx->motion.mvp_l1_flag = mvp_l1_flag;

      logtrace(LogSlice,"prediction unit %d,%d, L1, refIdx=%d mvp_l1_flag:%d\n",
               x0,y0, tctx->motion.refIdx[1], mvp_l1_flag);
    }
  }



  decode_prediction_unit(tctx->decctx, tctx->shdr, tctx->img, tctx->motion,
                         xC,yC,xB,yB, nCS, nPbW,nPbH, partIdx);
}