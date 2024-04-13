Proto_EndElement(GMarkupParseContext *parseContext,
                 const gchar *elementName,
                 gpointer userData,
                 GError **error)
{
   ProtoRequest *req = (ProtoRequest *) userData;

#if VGAUTH_PROTO_TRACE
   Debug("%s: elementName '%s'\n", __FUNCTION__, elementName);
#endif

   switch (req->parseState) {
   case PARSE_STATE_SEQNO:
   case PARSE_STATE_REQNAME:
   case PARSE_STATE_VERSION:
   case PARSE_STATE_USERNAME:
   case PARSE_STATE_ADDLINK:
   case PARSE_STATE_TICKET:
   case PARSE_STATE_PID:
   case PARSE_STATE_TOKEN:
   case PARSE_STATE_SAMLTOKEN:
   case PARSE_STATE_SAML_VALIDATE_ONLY:
   case PARSE_STATE_USERHANDLEINFO:
      req->parseState = PARSE_STATE_REQUEST;
      break;
   case PARSE_STATE_ALIASINFO:
      if (req->reqType == PROTO_REQUEST_ADDALIAS) {
         req->parseState = PARSE_STATE_REQUEST;
      } else if (req->reqType == PROTO_REQUEST_CREATETICKET) {
         req->parseState = PARSE_STATE_USERHANDLESAMLINFO;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Bad parse state, popping aliasInfo in req type %d",
                     req->reqType);
      }
      break;
   case PARSE_STATE_REQUEST:
      req->complete = TRUE;
      req->parseState = PARSE_STATE_NONE;
      break;
   case PARSE_STATE_PEMCERT:
      if (req->reqType == PROTO_REQUEST_ADDALIAS) {
         req->parseState = PARSE_STATE_REQUEST;
      } else if (req->reqType == PROTO_REQUEST_REMOVEALIAS) {
         req->parseState = PARSE_STATE_REQUEST;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Bad parse state, popping pemCert in req type %d",
                     req->reqType);
      }
      break;
   case PARSE_STATE_COMMENT:
      req->parseState = PARSE_STATE_ALIASINFO;
      break;
   case PARSE_STATE_SUBJECT:
   case PARSE_STATE_ANYSUBJECT:
      if (req->reqType == PROTO_REQUEST_ADDALIAS) {
         req->parseState = PARSE_STATE_ALIASINFO;
      } else if (req->reqType == PROTO_REQUEST_REMOVEALIAS) {
         req->parseState = PARSE_STATE_REQUEST;
      } else if (req->reqType == PROTO_REQUEST_CREATETICKET) {
         req->parseState = PARSE_STATE_ALIASINFO;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Bad parse state, popping (any)subject state %d",
                     req->parseState);
      }
      break;
   case PARSE_STATE_USERHANDLESAMLINFO:
   case PARSE_STATE_USERHANDLETYPE:
      req->parseState = PARSE_STATE_USERHANDLEINFO;
      break;
   case PARSE_STATE_USERHANDLESAMLSUBJECT:
      req->parseState = PARSE_STATE_USERHANDLESAMLINFO;
      break;
   default:
      g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                  "Bad parse state, popping unknown parse state %d",
                  req->parseState);
      ASSERT(0);
   }

}