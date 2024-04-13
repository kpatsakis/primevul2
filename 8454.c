static enum PartMode decode_part_mode(thread_context* tctx,
				      enum PredMode pred_mode, int cLog2CbSize)
{
  de265_image* img = tctx->img;

  if (pred_mode == MODE_INTRA) {
    logtrace(LogSlice,"# part_mode (INTRA)\n");

    int bit = decode_CABAC_bit(&tctx->cabac_decoder, &tctx->ctx_model[CONTEXT_MODEL_PART_MODE]);

    logtrace(LogSlice,"> %s\n",bit ? "2Nx2N" : "NxN");

    logtrace(LogSymbols,"$1 part_mode=%d\n",bit ? PART_2Nx2N : PART_NxN);

    return bit ? PART_2Nx2N : PART_NxN;
  }
  else {
    const seq_parameter_set& sps = img->get_sps();

    int bit0 = decode_CABAC_bit(&tctx->cabac_decoder, &tctx->ctx_model[CONTEXT_MODEL_PART_MODE+0]);
    if (bit0) { logtrace(LogSymbols,"$1 part_mode=%d\n",PART_2Nx2N); return PART_2Nx2N; }

    // CHECK_ME: I optimize code and fix bug here, need more VERIFY!
    int bit1 = decode_CABAC_bit(&tctx->cabac_decoder, &tctx->ctx_model[CONTEXT_MODEL_PART_MODE+1]);
    if (cLog2CbSize > sps.Log2MinCbSizeY) {
      if (!sps.amp_enabled_flag) {
        logtrace(LogSymbols,"$1 part_mode=%d\n",bit1 ? PART_2NxN : PART_Nx2N);
        return bit1 ? PART_2NxN : PART_Nx2N;
      }
      else {
        int bit3 = decode_CABAC_bit(&tctx->cabac_decoder, &tctx->ctx_model[CONTEXT_MODEL_PART_MODE+3]);
        if (bit3) {
          logtrace(LogSymbols,"$1 part_mode=%d\n",bit1 ? PART_2NxN : PART_Nx2N);
          return bit1 ? PART_2NxN : PART_Nx2N;
        }

        int bit4 = decode_CABAC_bypass(&tctx->cabac_decoder);
        if ( bit1 &&  bit4) {
          logtrace(LogSymbols,"$1 part_mode=%d\n",PART_2NxnD);
          return PART_2NxnD;
        }
        if ( bit1 && !bit4) {
          logtrace(LogSymbols,"$1 part_mode=%d\n",PART_2NxnU);
          return PART_2NxnU;
        }
        if (!bit1 && !bit4) {
          logtrace(LogSymbols,"$1 part_mode=%d\n",PART_nLx2N);
          return PART_nLx2N;
        }
        if (!bit1 &&  bit4) {
          logtrace(LogSymbols,"$1 part_mode=%d\n",PART_nRx2N);
          return PART_nRx2N;
        }
      }
    }
    else {
      // TODO, we could save one if here when first decoding the next bin and then
      // checkcLog2CbSize==3 when it is '0'

      if (bit1) {
        logtrace(LogSymbols,"$1 part_mode=%d\n",PART_2NxN);
        return PART_2NxN;
      }

      if (cLog2CbSize==3) {
        logtrace(LogSymbols,"$1 part_mode=%d\n",PART_Nx2N);
        return PART_Nx2N;
      }
      else {
        int bit2 = decode_CABAC_bit(&tctx->cabac_decoder, &tctx->ctx_model[CONTEXT_MODEL_PART_MODE+2]);
        logtrace(LogSymbols,"$1 part_mode=%d\n",PART_NxN-bit2);
        return (enum PartMode)((int)PART_NxN - bit2)/*bit2 ? PART_Nx2N : PART_NxN*/;
      }
    }
  }

  assert(false); // should never be reached
  return PART_2Nx2N;
}