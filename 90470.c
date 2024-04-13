AAC_DECODER_ERROR aacDecoder_drcSetParam (
    HANDLE_AAC_DRC    self,
    AACDEC_DRC_PARAM  param,
    INT               value )
{
  AAC_DECODER_ERROR ErrorStatus = AAC_DEC_OK;

 switch (param)
 {
 case DRC_CUT_SCALE:
 /* set attenuation scale factor */
 if ( (value < 0)
 || (value > DRC_MAX_QUANT_FACTOR) ) {
 return AAC_DEC_SET_PARAM_FAIL;
 }
 if (self == NULL) {
 return AAC_DEC_INVALID_HANDLE;
 }
    self->params.usrCut = (FIXP_DBL)((INT)(DRC_PARAM_QUANT_STEP>>DRC_PARAM_SCALE) * (INT)value);
 if (self->params.applyHeavyCompression == 0)
      self->params.cut = self->params.usrCut;
 break;
 case DRC_BOOST_SCALE:
 /* set boost factor */
 if ( (value < 0)
 || (value > DRC_MAX_QUANT_FACTOR) ) {
 return AAC_DEC_SET_PARAM_FAIL;
 }
 if (self == NULL) {
 return AAC_DEC_INVALID_HANDLE;
 }
    self->params.usrBoost = (FIXP_DBL)((INT)(DRC_PARAM_QUANT_STEP>>DRC_PARAM_SCALE) * (INT)value);
 if (self->params.applyHeavyCompression == 0)
      self->params.boost = self->params.usrBoost;
 break;
 case TARGET_REF_LEVEL:
 if ( value >  MAX_REFERENCE_LEVEL
 || value < -MAX_REFERENCE_LEVEL ) {
 return AAC_DEC_SET_PARAM_FAIL;
 }
 if (self == NULL) {
 return AAC_DEC_INVALID_HANDLE;
 }
 if (value < 0) {
      self->params.applyDigitalNorm = 0;
      self->params.targetRefLevel = -1;
 }
 else {
 /* ref_level must be between 0 and MAX_REFERENCE_LEVEL, inclusive */
      self->params.applyDigitalNorm = 1;
 if (self->params.targetRefLevel != (SCHAR)value) {
        self->params.targetRefLevel = (SCHAR)value;
        self->progRefLevel = (SCHAR)value; /* Always set the program reference level equal to the
                                               target level according to 4.5.2.7.3 of ISO/IEC 14496-3. */
 }
 }
 break;
 case APPLY_NORMALIZATION:
 if (value < 0 || value > 1) {
 return AAC_DEC_SET_PARAM_FAIL;
 }
 if (self == NULL) {
 return AAC_DEC_INVALID_HANDLE;
 }
 /* Store new parameter value */
    self->params.applyDigitalNorm = (UCHAR)value;
 break;
 case APPLY_HEAVY_COMPRESSION:
 if (value < 0 || value > 1) {
 return AAC_DEC_SET_PARAM_FAIL;
 }
 if (self == NULL) {
 return AAC_DEC_INVALID_HANDLE;
 }
 if (self->params.applyHeavyCompression != (UCHAR)value) {
 if (value == 1) {
 /* Disable scaling of DRC values by setting the max values */
        self->params.boost = FL2FXCONST_DBL(1.0f/(float)(1<<DRC_PARAM_SCALE));
        self->params.cut   = FL2FXCONST_DBL(1.0f/(float)(1<<DRC_PARAM_SCALE));
 } else {
 /* Restore the user params */
        self->params.boost = self->params.usrBoost;
        self->params.cut   = self->params.usrCut;
 }
 /* Store new parameter value */
      self->params.applyHeavyCompression = (UCHAR)value;
 }
 break;
 case DRC_BS_DELAY:
 if (value < 0 || value > 1) {
 return AAC_DEC_SET_PARAM_FAIL;
 }
 if (self == NULL) {
 return AAC_DEC_INVALID_HANDLE;
 }
    self->params.bsDelayEnable = value;
 break;
 case DRC_DATA_EXPIRY_FRAME:
 if (self == NULL) {
 return AAC_DEC_INVALID_HANDLE;
 }
    self->params.expiryFrame = (UINT)value;
 break;
 default:
 return AAC_DEC_SET_PARAM_FAIL;
 } /* switch(param) */

 /* switch on/off processing */
  self->enable = ( (self->params.boost > (FIXP_DBL)0)
 || (self->params.cut   > (FIXP_DBL)0)
 || (self->params.applyHeavyCompression != 0)
 || (self->params.targetRefLevel >= 0) );


 return ErrorStatus;
}
