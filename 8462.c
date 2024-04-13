static int decode_coeff_abs_level_remaining(thread_context* tctx,
                                            int cRiceParam)
{
  logtrace(LogSlice,"# decode_coeff_abs_level_remaining\n");

  int prefix=-1;
  int codeword=0;
  do {
    prefix++;
    codeword = decode_CABAC_bypass(&tctx->cabac_decoder);

    if (prefix>MAX_PREFIX) {
      return 0; // TODO: error
    }
  }
  while (codeword);

  // prefix = nb. 1 bits

  int value;

  if (prefix <= 3) {
    // when code only TR part (level < TRMax)

    codeword = decode_CABAC_FL_bypass(&tctx->cabac_decoder, cRiceParam);
    value = (prefix<<cRiceParam) + codeword;
  }
  else {
    // Suffix coded with EGk. Note that the unary part of EGk is already
    // included in the 'prefix' counter above.

    codeword = decode_CABAC_FL_bypass(&tctx->cabac_decoder, prefix-3+cRiceParam);
    value = (((1<<(prefix-3))+3-1)<<cRiceParam)+codeword;
  }

  logtrace(LogSymbols,"$1 coeff_abs_level_remaining=%d\n",value);

  return value;
}