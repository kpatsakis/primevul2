static int aacDecoder_drcParse (
    HANDLE_FDK_BITSTREAM  bs,
 CDrcPayload *pDrcBs,
    UINT                  payloadPosition )
{
 int i, numBands, bitCnt = 4;

 /* Move to the beginning of the DRC payload field */
 FDKpushBiDirectional(bs, FDKgetValidBits(bs)-payloadPosition);

 /* pce_tag_present */
 if (FDKreadBits(bs,1))
 {
    pDrcBs->pceInstanceTag = FDKreadBits(bs, 4); /* pce_instance_tag */
 /* only one program supported */
 FDKreadBits(bs, 4); /* drc_tag_reserved_bits */
    bitCnt += 8;
 } else {
    pDrcBs->pceInstanceTag = -1; /* not present */
 }

 if (FDKreadBits(bs,1)) { /* excluded_chns_present */
 /* get excluded_chn_mask */
    bitCnt += parseExcludedChannels(&pDrcBs->excludedChnsMask, bs);
 } else {
    pDrcBs->excludedChnsMask = 0;
 }

  numBands = 1;
 if (FDKreadBits(bs,1)) /* drc_bands_present */
 {
 /* get band_incr */
    numBands += FDKreadBits(bs, 4); /* drc_band_incr */
    pDrcBs->channelData.drcInterpolationScheme = FDKreadBits(bs, 4); /* drc_interpolation_scheme */
    bitCnt += 8;
 /* band_top */
 for (i = 0; i < numBands; i++)
 {
      pDrcBs->channelData.bandTop[i] = FDKreadBits(bs, 8); /* drc_band_top[i] */
      bitCnt += 8;
 }
 }
 else {
    pDrcBs->channelData.bandTop[0] = (1024 >> 2) - 1; /* ... comprising the whole spectrum. */;
 }

  pDrcBs->channelData.numBands = numBands;

 if (FDKreadBits(bs,1)) /* prog_ref_level_present */
 {
    pDrcBs->progRefLevel = FDKreadBits(bs, 7); /* prog_ref_level */
 FDKreadBits(bs, 1); /* prog_ref_level_reserved_bits */
    bitCnt += 8;
 } else {
    pDrcBs->progRefLevel = -1;
 }

 for (i = 0; i < numBands; i++)
 {
    pDrcBs->channelData.drcValue[i] = FDKreadBits(bs, 1) << 7; /* dyn_rng_sgn[i] */
    pDrcBs->channelData.drcValue[i] |= FDKreadBits(bs, 7) & 0x7F; /* dyn_rng_ctl[i] */
    bitCnt += 8;
 }

 /* Set DRC payload type */
  pDrcBs->channelData.drcDataType = MPEG_DRC_EXT_DATA;

 return (bitCnt);
}
