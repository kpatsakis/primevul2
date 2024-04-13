void read_coding_unit(thread_context* tctx,
                      int x0, int y0,  // position of coding unit in frame
                      int log2CbSize,
                      int ctDepth)
{
  de265_image* img = tctx->img;
  const seq_parameter_set& sps = img->get_sps();
  const pic_parameter_set& pps = img->get_pps();
  slice_segment_header* shdr = tctx->shdr;

  logtrace(LogSlice,"- read_coding_unit %d;%d cbsize:%d\n",x0,y0,1<<log2CbSize);


  //QQprintf("- read_coding_unit %d;%d cbsize:%d\n",x0,y0,1<<log2CbSize);

  img->set_log2CbSize(x0,y0, log2CbSize, true);

  /* This is only required on corrupted input streams.
     It may happen that there are several slices in the image that overlap.
     In this case, flags would accumulate from both slices.
  */
  img->clear_split_transform_flags(x0,y0, log2CbSize);

  int nCbS = 1<<log2CbSize; // number of coding block samples

  decode_quantization_parameters(tctx, x0,y0, x0, y0);


  if (pps.transquant_bypass_enable_flag)
    {
      int transquant_bypass = decode_transquant_bypass_flag(tctx);

      tctx->cu_transquant_bypass_flag = transquant_bypass;

      if (transquant_bypass) {
        img->set_cu_transquant_bypass(x0,y0,log2CbSize);
      }
    }
  else {
    tctx->cu_transquant_bypass_flag = 0;
  }

  uint8_t cu_skip_flag = 0;
  if (shdr->slice_type != SLICE_TYPE_I) {
    cu_skip_flag = decode_cu_skip_flag(tctx,x0,y0,ctDepth);
  }

  int IntraSplitFlag = 0;

  enum PredMode cuPredMode;

  if (cu_skip_flag) {
    read_prediction_unit_SKIP(tctx,x0,y0,nCbS,nCbS);

    img->set_PartMode(x0,y0, PART_2Nx2N); // need this for deblocking filter
    img->set_pred_mode(x0,y0,log2CbSize, MODE_SKIP);
    cuPredMode = MODE_SKIP;

    logtrace(LogSlice,"CU pred mode: SKIP\n");


    // DECODE

    int nCS_L = 1<<log2CbSize;
    decode_prediction_unit(tctx->decctx,tctx->shdr,tctx->img,tctx->motion,
                           x0,y0, 0,0, nCS_L, nCS_L,nCS_L, 0);
  }
  else /* not skipped */ {
    if (shdr->slice_type != SLICE_TYPE_I) {
      int pred_mode_flag = decode_pred_mode_flag(tctx);
      cuPredMode = pred_mode_flag ? MODE_INTRA : MODE_INTER;
    }
    else {
      cuPredMode = MODE_INTRA;
    }

    img->set_pred_mode(x0,y0,log2CbSize, cuPredMode);

    logtrace(LogSlice,"CU pred mode: %s\n", cuPredMode==MODE_INTRA ? "INTRA" : "INTER");


    enum PartMode PartMode;

    if (cuPredMode != MODE_INTRA ||
        log2CbSize == sps.Log2MinCbSizeY) {
      PartMode = decode_part_mode(tctx, cuPredMode, log2CbSize);

      if (PartMode==PART_NxN && cuPredMode==MODE_INTRA) {
        IntraSplitFlag=1;
      }
    } else {
      PartMode = PART_2Nx2N;
    }

    img->set_PartMode(x0,y0, PartMode); // needed for deblocking ?

    logtrace(LogSlice, "PartMode: %s\n", part_mode_name(PartMode));


    bool pcm_flag = false;

    if (cuPredMode == MODE_INTRA) {
      if (PartMode == PART_2Nx2N && sps.pcm_enabled_flag &&
          log2CbSize >= sps.Log2MinIpcmCbSizeY &&
          log2CbSize <= sps.Log2MaxIpcmCbSizeY) {
        pcm_flag = decode_CABAC_term_bit(&tctx->cabac_decoder);
      }

      if (pcm_flag) {
        img->set_pcm_flag(x0,y0,log2CbSize);

        read_pcm_samples(tctx, x0,y0, log2CbSize);
      }
      else {
        int pbOffset = (PartMode == PART_NxN) ? (nCbS/2) : nCbS;
        int log2IntraPredSize = (PartMode == PART_NxN) ? (log2CbSize-1) : log2CbSize;

        logtrace(LogSlice,"nCbS:%d pbOffset:%d\n",nCbS,pbOffset);

        int prev_intra_luma_pred_flag[4];

        int idx=0;
        for (int j=0;j<nCbS;j+=pbOffset)
          for (int i=0;i<nCbS;i+=pbOffset)
            {
              prev_intra_luma_pred_flag[idx++] = decode_prev_intra_luma_pred_flag(tctx);
            }

        int mpm_idx[4], rem_intra_luma_pred_mode[4];
        idx=0;

        int availableA0 = check_CTB_available(img, x0,y0, x0-1,y0);
        int availableB0 = check_CTB_available(img, x0,y0, x0,y0-1);

        for (int j=0;j<nCbS;j+=pbOffset)
          for (int i=0;i<nCbS;i+=pbOffset)
            {
              if (prev_intra_luma_pred_flag[idx]) {
                mpm_idx[idx] = decode_mpm_idx(tctx);
              }
              else {
                rem_intra_luma_pred_mode[idx] = decode_rem_intra_luma_pred_mode(tctx);
              }


              int x = x0+i;
              int y = y0+j;

              // --- find intra prediction mode ---

              int IntraPredMode;

              int availableA = availableA0 || (i>0); // left candidate always available for right blk
              int availableB = availableB0 || (j>0); // top candidate always available for bottom blk



              int PUidx = (x>>sps.Log2MinPUSize) + (y>>sps.Log2MinPUSize)*sps.PicWidthInMinPUs;

              enum IntraPredMode candModeList[3];

              fillIntraPredModeCandidates(candModeList,x,y,PUidx,
                                          availableA, availableB, img);

              for (int i=0;i<3;i++)
                logtrace(LogSlice,"candModeList[%d] = %d\n", i, candModeList[i]);

              if (prev_intra_luma_pred_flag[idx]==1) {
                IntraPredMode = candModeList[ mpm_idx[idx] ];
              }
              else {
                // sort candModeList

                if (candModeList[0] > candModeList[1]) {
                  std::swap(candModeList[0],candModeList[1]);
                }
                if (candModeList[0] > candModeList[2]) {
                  std::swap(candModeList[0],candModeList[2]);
                }
                if (candModeList[1] > candModeList[2]) {
                  std::swap(candModeList[1],candModeList[2]);
                }

                // skip modes in the list
                // (we have 35 modes. skipping the 3 in the list gives us 32, which can be selected by 5 bits)
                IntraPredMode = rem_intra_luma_pred_mode[idx];
                for (int n=0;n<=2;n++) {
                  if (IntraPredMode >= candModeList[n]) { IntraPredMode++; }
                }
              }

              logtrace(LogSlice,"IntraPredMode[%d][%d] = %d (log2blk:%d)\n",x,y,IntraPredMode, log2IntraPredSize);

              img->set_IntraPredMode(PUidx, log2IntraPredSize,
                                     (enum IntraPredMode)IntraPredMode);

              idx++;
            }


        // set chroma intra prediction mode

        if (sps.ChromaArrayType == CHROMA_444) {
          // chroma 4:4:4

          idx = 0;
          for (int j=0;j<nCbS;j+=pbOffset)
            for (int i=0;i<nCbS;i+=pbOffset) {
              int x = x0+i;
              int y = y0+j;

              int intra_chroma_pred_mode = decode_intra_chroma_pred_mode(tctx);
              int IntraPredMode = img->get_IntraPredMode(x,y);

              int IntraPredModeC = map_chroma_pred_mode(intra_chroma_pred_mode, IntraPredMode);

              logtrace(LogSlice,"IntraPredModeC[%d][%d]: %d (blksize:%d)\n",x,y,IntraPredModeC,
                       1<<log2IntraPredSize);

              img->set_IntraPredModeC(x,y, log2IntraPredSize,
                                      (enum IntraPredMode)IntraPredModeC,
                                      intra_chroma_pred_mode == 4);
              idx++;
            }
        }
        else if (sps.ChromaArrayType != CHROMA_MONO) {
          // chroma 4:2:0 and 4:2:2

          int intra_chroma_pred_mode = decode_intra_chroma_pred_mode(tctx);
          int IntraPredMode = img->get_IntraPredMode(x0,y0);
          logtrace(LogSlice,"IntraPredMode: %d\n",IntraPredMode);
          int IntraPredModeC = map_chroma_pred_mode(intra_chroma_pred_mode, IntraPredMode);

          if (sps.ChromaArrayType == CHROMA_422) {
            IntraPredModeC = map_chroma_422[ IntraPredModeC ];
          }

          img->set_IntraPredModeC(x0,y0, log2CbSize,
                                  (enum IntraPredMode)IntraPredModeC,
                                  intra_chroma_pred_mode == 4);
        }
      }
    }
    else { // INTER
      int nCS = 1<<log2CbSize;

      if (PartMode == PART_2Nx2N) {
        read_prediction_unit(tctx,x0,y0,0,0,nCbS,nCbS,ctDepth,nCS,0);
      }
      else if (PartMode == PART_2NxN) {
        read_prediction_unit(tctx,x0,y0,0,0     ,nCbS,nCbS/2,ctDepth,nCS,0);
        read_prediction_unit(tctx,x0,y0,0,nCbS/2,nCbS,nCbS/2,ctDepth,nCS,1);
      }
      else if (PartMode == PART_Nx2N) {
        read_prediction_unit(tctx,x0,y0,0,0  ,   nCbS/2,nCbS,ctDepth,nCS,0);
        read_prediction_unit(tctx,x0,y0,nCbS/2,0,nCbS/2,nCbS,ctDepth,nCS,1);
      }
      else if (PartMode == PART_2NxnU) {
        read_prediction_unit(tctx,x0,y0,0,0,     nCbS,nCbS/4,ctDepth,nCS,0);
        read_prediction_unit(tctx,x0,y0,0,nCbS/4,nCbS,nCbS*3/4,ctDepth,nCS,1);
      }
      else if (PartMode == PART_2NxnD) {
        read_prediction_unit(tctx,x0,y0,0,0,       nCbS,nCbS*3/4,ctDepth,nCS,0);
        read_prediction_unit(tctx,x0,y0,0,nCbS*3/4,nCbS,nCbS/4,ctDepth,nCS,1);
      }
      else if (PartMode == PART_nLx2N) {
        read_prediction_unit(tctx,x0,y0,0,0,     nCbS/4,nCbS,ctDepth,nCS,0);
        read_prediction_unit(tctx,x0,y0,nCbS/4,0,nCbS*3/4,nCbS,ctDepth,nCS,1);
      }
      else if (PartMode == PART_nRx2N) {
        read_prediction_unit(tctx,x0,y0,0,0,       nCbS*3/4,nCbS,ctDepth,nCS,0);
        read_prediction_unit(tctx,x0,y0,nCbS*3/4,0,nCbS/4,nCbS,ctDepth,nCS,1);
      }
      else if (PartMode == PART_NxN) {
        read_prediction_unit(tctx,x0,y0,0,0,          nCbS/2,nCbS/2,ctDepth,nCS,0);
        read_prediction_unit(tctx,x0,y0,nCbS/2,0,     nCbS/2,nCbS/2,ctDepth,nCS,1);
        read_prediction_unit(tctx,x0,y0,0,nCbS/2,     nCbS/2,nCbS/2,ctDepth,nCS,2);
        read_prediction_unit(tctx,x0,y0,nCbS/2,nCbS/2,nCbS/2,nCbS/2,ctDepth,nCS,3);
      }
      else {
        assert(0); // undefined PartMode
      }
    } // INTER


    // decode residual

    if (!pcm_flag) { // !pcm
      bool rqt_root_cbf;

      uint8_t merge_flag = tctx->motion.merge_flag; // !!get_merge_flag(ctx,x0,y0);

      if (cuPredMode != MODE_INTRA &&
          !(PartMode == PART_2Nx2N && merge_flag)) {

        rqt_root_cbf = !!decode_rqt_root_cbf(tctx);
      }
      else {
        /* rqt_root_cbf=1 is inferred for Inter blocks with 2Nx2N, merge mode.
           These must be some residual data, because otherwise, the CB could
           also be coded in SKIP mode.
         */

        rqt_root_cbf = true;
      }

      //set_rqt_root_cbf(ctx,x0,y0, log2CbSize, rqt_root_cbf);

      if (rqt_root_cbf) {
        int MaxTrafoDepth;

        if (cuPredMode==MODE_INTRA) {
          MaxTrafoDepth = sps.max_transform_hierarchy_depth_intra + IntraSplitFlag;
        }
        else {
          MaxTrafoDepth = sps.max_transform_hierarchy_depth_inter;
        }

        logtrace(LogSlice,"MaxTrafoDepth: %d\n",MaxTrafoDepth);

        uint8_t initial_chroma_cbf = 1;
        if (sps.ChromaArrayType == CHROMA_MONO) {
          initial_chroma_cbf = 0;
        }

        read_transform_tree(tctx, x0,y0, x0,y0, x0,y0, log2CbSize, 0,0,
                            MaxTrafoDepth, IntraSplitFlag, cuPredMode,
                            initial_chroma_cbf, initial_chroma_cbf);
      }
    } // !pcm
  }
}