void aacDecoder_drcApply (
        HANDLE_AAC_DRC          self,
 void *pSbrDec,
 CAacDecoderChannelInfo *pAacDecoderChannelInfo,
 CDrcChannelData *pDrcChData,
        FIXP_DBL               *extGain,
 int  ch, /* needed only for SBR */
 int  aacFrameSize,
 int  bSbrPresent )
{
 int band, top, bin, numBands;
 int bottom = 0;
 int modifyBins = 0;

  FIXP_DBL max_mantissa;
  INT max_exponent;

  FIXP_DBL norm_mantissa = FL2FXCONST_DBL(0.5f);
  INT  norm_exponent = 1;

  FIXP_DBL fact_mantissa[MAX_DRC_BANDS];
  INT  fact_exponent[MAX_DRC_BANDS];

 CDrcParams *pParams = &self->params;

  FIXP_DBL    *pSpectralCoefficient  =  SPEC_LONG(pAacDecoderChannelInfo->pSpectralCoefficient);
 CIcsInfo *pIcsInfo              = &pAacDecoderChannelInfo->icsInfo;
  SHORT       *pSpecScale            =  pAacDecoderChannelInfo->specScale;

 int winSeq = pIcsInfo->WindowSequence;

 /* Increment and check expiry counter */
 if ( (pParams->expiryFrame > 0)
 && (++pDrcChData->expiryCount > pParams->expiryFrame) )
 { /* The DRC data is too old, so delete it. */
    aacDecoder_drcInitChannelData( pDrcChData );
 }

 if (!self->enable) {
    sbrDecoder_drcDisable( (HANDLE_SBRDECODER)pSbrDec, ch );
 if (extGain != NULL) {
      INT gainScale = (INT)*extGain;
 /* The gain scaling must be passed to the function in the buffer pointed on by extGain. */
 if (gainScale >= 0 && gainScale <= DFRACT_BITS) {
 *extGain = scaleValue(norm_mantissa, norm_exponent-gainScale);
 } else {
        FDK_ASSERT(0);
 }
 }
 return;
 }

  numBands = pDrcChData->numBands;
  top = FDKmax(0, numBands-1);

  pDrcChData->bandTop[0] = fixMin(pDrcChData->bandTop[0], (aacFrameSize >> 2) - 1);

 /* If program reference normalization is done in the digital domain,
  modify factor to perform normalization.  prog_ref_level can
  alternatively be passed to the system for modification of the level in
  the analog domain.  Analog level modification avoids problems with
  reduced DAC SNR (if signal is attenuated) or clipping (if signal is
  boosted) */

 if (pParams->targetRefLevel >= 0)
 {
 /* 0.5^((targetRefLevel - progRefLevel)/24) */
    norm_mantissa = fLdPow(
            FL2FXCONST_DBL(-1.0), /* log2(0.5) */
 0,
 (FIXP_DBL)((INT)(FL2FXCONST_DBL(1.0f/24.0)>>3) * (INT)(pParams->targetRefLevel-self->progRefLevel)),
 3,
 &norm_exponent );
 }
 /* Always export the normalization gain (if possible). */
 if (extGain != NULL) {
    INT gainScale = (INT)*extGain;
 /* The gain scaling must be passed to the function in the buffer pointed on by extGain. */
 if (gainScale >= 0 && gainScale <= DFRACT_BITS) {
 *extGain = scaleValue(norm_mantissa, norm_exponent-gainScale);
 } else {
      FDK_ASSERT(0);
 }
 }
 if (self->params.applyDigitalNorm == 0) {
 /* Reset normalization gain since this module must not apply it */
    norm_mantissa = FL2FXCONST_DBL(0.5f);
    norm_exponent = 1;
 }


 /* calc scale factors */
 for (band = 0; band < numBands; band++)
 {
    UCHAR drcVal = pDrcChData->drcValue[band];
    top = fixMin((int)( (pDrcChData->bandTop[band]+1)<<2 ), aacFrameSize);

    fact_mantissa[band] = FL2FXCONST_DBL(0.5f);
    fact_exponent[band] = 1;

 if (  pParams->applyHeavyCompression
 && ((AACDEC_DRC_PAYLOAD_TYPE)pDrcChData->drcDataType == DVB_DRC_ANC_DATA) )
 {
      INT compressionFactorVal_e;
 int valX, valY;

      valX = drcVal >> 4;
      valY = drcVal & 0x0F;

 /* calculate the unscaled heavy compression factor.
         compressionFactor = 48.164 - 6.0206*valX - 0.4014*valY dB
         range: -48.166 dB to 48.164 dB */
 if ( drcVal != 0x7F ) {
        fact_mantissa[band] =
          fPowInt( FL2FXCONST_DBL(0.95483867181), /* -0.4014dB = 0.95483867181 */
 0,
                   valY,
 &compressionFactorVal_e );

 /* -0.0008dB (48.164 - 6.0206*8 = -0.0008) */
        fact_mantissa[band] = fMult(FL2FXCONST_DBL(0.99990790084), fact_mantissa[band]);

        fact_exponent[band] = DVB_COMPRESSION_SCALE - valX + compressionFactorVal_e;
 }
 } else
 if ((AACDEC_DRC_PAYLOAD_TYPE)pDrcChData->drcDataType == MPEG_DRC_EXT_DATA)
 {
 /* apply the scaled dynamic range control words to factor.
     * if scaling drc_cut (or drc_boost), or control word drc_mantissa is 0
     * then there is no dynamic range compression
     *
     * if pDrcChData->drcSgn[band] is 
     *  1 then gain is < 1 :  factor = 2^(-self->cut   * pDrcChData->drcMag[band] / 24)
     *  0 then gain is > 1 :  factor = 2^( self->boost * pDrcChData->drcMag[band] / 24)
     */

 if ((drcVal&0x7F) > 0) {
      FIXP_DBL tParamVal = (drcVal & 0x80) ? -pParams->cut : pParams->boost;

      fact_mantissa[band] =
        f2Pow( (FIXP_DBL)((INT)fMult(FL2FXCONST_DBL(1.0f/192.0f), tParamVal) * (drcVal&0x7F)),
 3+DRC_PARAM_SCALE,
 &fact_exponent[band] );
 }
 }

    fact_mantissa[band] = fMult(fact_mantissa[band], norm_mantissa);
    fact_exponent[band] += norm_exponent;


    bottom = top;

 } /* end loop over bands */


 /* normalizations */
 {
 int res;

    max_mantissa = FL2FXCONST_DBL(0.0f);
    max_exponent = 0;
 for (band = 0; band < numBands; band++) {
      max_mantissa = fixMax(max_mantissa, fact_mantissa[band]);
      max_exponent = fixMax(max_exponent, fact_exponent[band]);
 }

 /* left shift factors to gain accurancy */
    res = CntLeadingZeros(max_mantissa) - 1;

 /* above topmost DRC band gain factor is 1 */
 if (((pDrcChData->bandTop[numBands-1]+1)<<2) < aacFrameSize) res = 0;

 if (res > 0) {
      res = fixMin(res, max_exponent);
      max_exponent -= res;

 for (band = 0; band < numBands; band++) {
        fact_mantissa[band] <<= res;
        fact_exponent[band] -= res;
 }
 }

 /* normalize magnitudes to one scale factor */
 for (band = 0; band < numBands; band++) {
 if (fact_exponent[band] < max_exponent) {
        fact_mantissa[band] >>= max_exponent - fact_exponent[band];
 }
 if (fact_mantissa[band] != FL2FXCONST_DBL(0.5f)) {
        modifyBins = 1;
 }
 }
 if (max_exponent != 1) {
      modifyBins = 1;
 }
 }

 /*  apply factor to spectral lines
   *  short blocks must take care that bands fall on 
   *  block boundaries!
   */
 if (!bSbrPresent)
 {
    bottom = 0;

 if (!modifyBins) {
 /* We don't have to modify the spectral bins because the fractional part of all factors is 0.5.
         In order to keep accurancy we don't apply the factor but decrease the exponent instead. */
      max_exponent -= 1;
 } else
 {
 for (band = 0; band < numBands; band++)
 {
        top = fixMin((int)( (pDrcChData->bandTop[band]+1)<<2 ), aacFrameSize); /* ... * DRC_BAND_MULT; */

 for (bin = bottom; bin < top; bin++) {
          pSpectralCoefficient[bin] = fMult(pSpectralCoefficient[bin], fact_mantissa[band]);
 }

        bottom = top;
 }
 }

 /* above topmost DRC band gain factor is 1 */
 if (max_exponent > 0) {
 for (bin = bottom; bin < aacFrameSize; bin+=1) {
        pSpectralCoefficient[bin] >>= max_exponent;
 }
 }

 /* adjust scaling */
    pSpecScale[0] += max_exponent;

 if (winSeq == EightShortSequence) {
 int win;
 for (win = 1; win < 8; win++) {
        pSpecScale[win] += max_exponent;
 }
 }
 }
 else {
    HANDLE_SBRDECODER hSbrDecoder = (HANDLE_SBRDECODER)pSbrDec;
    UINT numBands = pDrcChData->numBands;

 /* feed factors into SBR decoder for application in QMF domain. */
    sbrDecoder_drcFeedChannel (
            hSbrDecoder,
            ch,
            numBands,
            fact_mantissa,
            max_exponent,
            pDrcChData->drcInterpolationScheme,
            winSeq,
            pDrcChData->bandTop
 );
 }

 return;
}
