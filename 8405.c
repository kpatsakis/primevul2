void read_transform_tree(thread_context* tctx,
                         int x0, int y0,        // position of TU in frame
                         int xBase, int yBase,  // position of parent TU in frame
                         int xCUBase, int yCUBase, // position of CU in frame
                         int log2TrafoSize,
                         int trafoDepth,
                         int blkIdx,
                         int MaxTrafoDepth,
                         int IntraSplitFlag,
                         enum PredMode cuPredMode,
                         uint8_t parent_cbf_cb,uint8_t parent_cbf_cr)
{
  logtrace(LogSlice,"- read_transform_tree (interleaved) x0:%d y0:%d xBase:%d yBase:%d "
           "log2TrafoSize:%d trafoDepth:%d MaxTrafoDepth:%d parent-cbf-cb:%d parent-cbf-cr:%d\n",
           x0,y0,xBase,yBase,log2TrafoSize,trafoDepth,MaxTrafoDepth,parent_cbf_cb,parent_cbf_cr);

  de265_image* img = tctx->img;
  const seq_parameter_set& sps = img->get_sps();

  int split_transform_flag;

  enum PredMode PredMode = img->get_pred_mode(x0,y0);
  assert(PredMode == cuPredMode);

  /*  If TrafoSize is larger than maximum size   -> split automatically
      If TrafoSize is at minimum size            -> do not split
      If maximum transformation depth is reached -> do not split
      If intra-prediction is NxN mode            -> split automatically (only at level 0)
      Otherwise  ->  read split flag
  */
  if (log2TrafoSize <= sps.Log2MaxTrafoSize &&
      log2TrafoSize >  sps.Log2MinTrafoSize &&
      trafoDepth < MaxTrafoDepth &&
      !(IntraSplitFlag && trafoDepth==0))
    {
      split_transform_flag = decode_split_transform_flag(tctx, log2TrafoSize);
    }
  else
    {
      enum PartMode PartMode = img->get_PartMode(x0,y0);

      int interSplitFlag= (sps.max_transform_hierarchy_depth_inter==0 &&
                           trafoDepth == 0 &&
                           PredMode == MODE_INTER &&
                           PartMode != PART_2Nx2N);

      split_transform_flag = (log2TrafoSize > sps.Log2MaxTrafoSize ||
                              (IntraSplitFlag==1 && trafoDepth==0) ||
                              interSplitFlag==1) ? 1:0;
    }

  if (split_transform_flag) {
    logtrace(LogSlice,"set_split_transform_flag(%d,%d, %d)\n",x0,y0,trafoDepth);
    img->set_split_transform_flag(x0,y0,trafoDepth);
  }

  int cbf_cb=-1;
  int cbf_cr=-1;

  // CBF_CB/CR flags are encoded like this:
  // 4:2:0 and 4:4:4 modes: binary flag in bit 0
  // 4:2:2 mode: bit 0: top block, bit 1: bottom block

  if ((log2TrafoSize>2 && sps.ChromaArrayType != CHROMA_MONO) ||
      sps.ChromaArrayType == CHROMA_444) {
    // we do not have to test for trafoDepth==0, because parent_cbf_cb is 1 at depth 0
    if (/*trafoDepth==0 ||*/ parent_cbf_cb) {
      cbf_cb = decode_cbf_chroma(tctx,trafoDepth);

      if (sps.ChromaArrayType == CHROMA_422 && (!split_transform_flag || log2TrafoSize==3)) {
        cbf_cb |= (decode_cbf_chroma(tctx,trafoDepth) << 1);
      }
    }

    // we do not have to test for trafoDepth==0, because parent_cbf_cb is 1 at depth 0
    if (/*trafoDepth==0 ||*/ parent_cbf_cr) {
      cbf_cr = decode_cbf_chroma(tctx,trafoDepth);

      if (sps.ChromaArrayType == CHROMA_422 && (!split_transform_flag || log2TrafoSize==3)) {
        cbf_cr |= (decode_cbf_chroma(tctx,trafoDepth) << 1);
      }
    }
  }

  //printf("CBF: cb:%d cr:%d\n",cbf_cb,cbf_cr);

  // cbf_cr/cbf_cb not present in bitstream -> induce values

  if (cbf_cb<0) {
    assert(!(trafoDepth==0 && log2TrafoSize==2));

    /* The standard specifies to check trafoDepth>0 AND log2TrafoSize==2.
       However, I think that trafoDepth>0 is redundant as a CB is always
       at least 8x8 and hence trafoDepth>0.
    */

    if (trafoDepth>0 && log2TrafoSize==2) {
      cbf_cb = parent_cbf_cb;
    } else {
      cbf_cb=0;
    }
  }

  if (cbf_cr<0) {
    if (trafoDepth>0 && log2TrafoSize==2) {
      cbf_cr = parent_cbf_cr;
    } else {
      cbf_cr=0;
    }
  }

  if (split_transform_flag) {
    int x1 = x0 + (1<<(log2TrafoSize-1));
    int y1 = y0 + (1<<(log2TrafoSize-1));

    logtrace(LogSlice,"transform split.\n");

    read_transform_tree(tctx, x0,y0, x0,y0, xCUBase,yCUBase, log2TrafoSize-1, trafoDepth+1, 0,
                        MaxTrafoDepth,IntraSplitFlag, cuPredMode, cbf_cb,cbf_cr);
    read_transform_tree(tctx, x1,y0, x0,y0, xCUBase,yCUBase, log2TrafoSize-1, trafoDepth+1, 1,
                        MaxTrafoDepth,IntraSplitFlag, cuPredMode, cbf_cb,cbf_cr);
    read_transform_tree(tctx, x0,y1, x0,y0, xCUBase,yCUBase, log2TrafoSize-1, trafoDepth+1, 2,
                        MaxTrafoDepth,IntraSplitFlag, cuPredMode, cbf_cb,cbf_cr);
    read_transform_tree(tctx, x1,y1, x0,y0, xCUBase,yCUBase, log2TrafoSize-1, trafoDepth+1, 3,
                        MaxTrafoDepth,IntraSplitFlag, cuPredMode, cbf_cb,cbf_cr);
  }
  else {
    int cbf_luma;

    if (PredMode==MODE_INTRA || trafoDepth!=0 || cbf_cb || cbf_cr) {
      cbf_luma = decode_cbf_luma(tctx,trafoDepth);
    }
    else {
      /* There cannot be INTER blocks with no residual data.
         That case is already handled with rqt_root_cbf.
      */

      cbf_luma = 1;
    }

    logtrace(LogSlice,"call read_transform_unit %d/%d\n",x0,y0);

    read_transform_unit(tctx, x0,y0,xBase,yBase, xCUBase,yCUBase, log2TrafoSize,trafoDepth, blkIdx,
                        cbf_luma, cbf_cb, cbf_cr);
  }
}