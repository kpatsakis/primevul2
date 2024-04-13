ProtoValidationTypeString(const ServiceValidationResultsType t)
{
   switch (t) {
   case VALIDATION_RESULTS_TYPE_NAMEPASSWORD:
      return VGAUTH_USERHANDLE_TYPE_NAMEPASSWORD;
   case VALIDATION_RESULTS_TYPE_SSPI:
      return VGAUTH_USERHANDLE_TYPE_SSPI;
   case VALIDATION_RESULTS_TYPE_SAML:
      return VGAUTH_USERHANDLE_TYPE_SAML;
   case VALIDATION_RESULTS_TYPE_SAML_INFO_ONLY:
      return VGAUTH_USERHANDLE_TYPE_SAML_INFO_ONLY;
   case VALIDATION_RESULTS_TYPE_UNKNOWN:
   default:
      ASSERT(0);
      Warning("%s: Tried to convert a validationType of %d to a string\n",
              __FUNCTION__, t);
      return "<UNKNOWN>";
   }
}