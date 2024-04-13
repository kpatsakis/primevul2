static inline status_t statusFromOMXError(int32_t omxError) {
 switch (omxError) {
 case OMX_ErrorInvalidComponentName:
 case OMX_ErrorComponentNotFound:
 return NAME_NOT_FOUND; // can trigger illegal argument error for provided names.
 default:
 return isOMXError(omxError) ? omxError : 0; // no translation required
 }
}
