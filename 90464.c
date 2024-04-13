void aacDecoder_drcInit (
    HANDLE_AAC_DRC self )
{
 CDrcParams *pParams;

 if (self == NULL) {
 return;
 }

 /* init control fields */
  self->enable = 0;
  self->numThreads = 0;

 /* init params */
  pParams = &self->params;
  pParams->bsDelayEnable = 0;
  pParams->cut      = FL2FXCONST_DBL(0.0f);
  pParams->usrCut   = FL2FXCONST_DBL(0.0f);
  pParams->boost    = FL2FXCONST_DBL(0.0f);
  pParams->usrBoost = FL2FXCONST_DBL(0.0f);
  pParams->targetRefLevel = -1;
  pParams->expiryFrame = AACDEC_DRC_DFLT_EXPIRY_FRAMES;
  pParams->applyDigitalNorm = 0;
  pParams->applyHeavyCompression = 0;

 /* initial program ref level = target ref level */
  self->progRefLevel = pParams->targetRefLevel;
  self->progRefLevelPresent = 0;
  self->presMode = -1;
}
