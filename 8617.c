Proto_TextContents(GMarkupParseContext *parseContext,
                   const gchar *text,
                   gsize textSize,
                   gpointer userData,
                   GError **error)
{
   ProtoRequest *req = (ProtoRequest *) userData;
   gchar *val;
   int iVal;
   gboolean duplicate_found = FALSE;

#if VGAUTH_PROTO_TRACE
   Debug("%s: parseState %d, text '%*s'\n", __FUNCTION__, req->parseState, (int) textSize, text);
#endif

   /*
    * Simple string values should be set only once, but a malicious client
    * could send them multiple times, which could cause a leak if not
    * checked.
    */
#define SET_CHECK_DUP(var, val) \
   if ((var) != NULL) { duplicate_found = TRUE; goto done; } \
   else { (var) = (val); }

   val = g_strndup(text, textSize);

   switch (req->parseState) {
   case PARSE_STATE_SEQNO:
      req->sequenceNumber = atoi(val);
      break;
   case PARSE_STATE_REQNAME:
      if (req->reqType != PROTO_REQUEST_UNKNOWN) {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Trying to handle new request of type %s when processing"
                     " a request of type %d",
                     val, req->reqType);
         goto done;
      }
      req->reqType = Proto_RequestNameToType(val);

      /*
       * Do any special init work once we've chosen the request type.
       */
      if (req->reqType == PROTO_REQUEST_REMOVEALIAS) {
         // init removeAlias to be UNSET, so that we handle the removeAll case
         req->reqData.removeAlias.subject.type = SUBJECT_TYPE_UNSET;
      }
      break;
   case PARSE_STATE_VERSION:
      if (req->reqType != PROTO_REQUEST_SESSION_REQ) {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found version number in req type %d",
                     req->reqType);
         goto done;
      }
      req->reqData.sessionReq.version = atoi(val);
      break;
   case PARSE_STATE_USERNAME:
      switch (req->reqType) {
      case PROTO_REQUEST_SESSION_REQ:
         SET_CHECK_DUP(req->reqData.sessionReq.userName, val);
         break;
      case PROTO_REQUEST_ADDALIAS:
         SET_CHECK_DUP(req->reqData.addAlias.userName, val);
         break;
      case PROTO_REQUEST_REMOVEALIAS:
         SET_CHECK_DUP(req->reqData.removeAlias.userName, val);
         break;
      case PROTO_REQUEST_QUERYALIASES:
         SET_CHECK_DUP(req->reqData.queryAliases.userName, val);
         break;
      case PROTO_REQUEST_CREATETICKET:
         SET_CHECK_DUP(req->reqData.createTicket.userName, val);
         break;
      case PROTO_REQUEST_VALIDATE_SAML_BEARER_TOKEN:
         SET_CHECK_DUP(req->reqData.validateSamlBToken.userName, val);
         break;
      default:
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found username in req type %d",
                     req->reqType);
         goto done;
      }
      val = NULL;
      break;
   case PARSE_STATE_TICKET:
      if (req->reqType == PROTO_REQUEST_VALIDATETICKET) {
         SET_CHECK_DUP(req->reqData.validateTicket.ticket, val);
      } else if (req->reqType == PROTO_REQUEST_REVOKETICKET) {
         SET_CHECK_DUP(req->reqData.revokeTicket.ticket, val);
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found ticket in req type %d",
                     req->reqType);
         goto done;
      }
      val = NULL;
      break;
   case PARSE_STATE_ADDLINK:
      if (req->reqType == PROTO_REQUEST_ADDALIAS) {
         req->reqData.addAlias.addMapped = ((atoi(val) == 1) ? TRUE : FALSE);
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found addMappedLink in req type %d",
                     req->reqType);
         goto done;
      }
      break;
   case PARSE_STATE_REQUEST:
   case PARSE_STATE_ALIASINFO:
   case PARSE_STATE_USERHANDLEINFO:
   case PARSE_STATE_USERHANDLESAMLINFO:
      /*
       * Should just be whitespace, ignore
       */
      break;
   case PARSE_STATE_SUBJECT:
      if (req->reqType == PROTO_REQUEST_ADDALIAS) {
         SET_CHECK_DUP(req->reqData.addAlias.aliasInfo.name, val);
         req->reqData.addAlias.aliasInfo.type = SUBJECT_TYPE_NAMED;
      } else if (req->reqType == PROTO_REQUEST_REMOVEALIAS) {
         SET_CHECK_DUP(req->reqData.removeAlias.subject.name, val);
         req->reqData.removeAlias.subject.type = SUBJECT_TYPE_NAMED;
      } else if (req->reqType == PROTO_REQUEST_CREATETICKET) {
         SET_CHECK_DUP(req->reqData.createTicket.samlData.aliasInfo.name, val);
         req->reqData.createTicket.samlData.aliasInfo.type = SUBJECT_TYPE_NAMED;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found SUBJECT in req type %d",
                     req->reqType);
         goto done;
      }
      val = NULL;
      break;
   case PARSE_STATE_ANYSUBJECT:
      /*
       * Don't expect to ever get here, but sombody may not use
       * an empty-element tag.
       */
      if (req->reqType == PROTO_REQUEST_ADDALIAS) {
         req->reqData.addAlias.aliasInfo.type = SUBJECT_TYPE_ANY;
         req->reqData.addAlias.aliasInfo.name = NULL;
      } else if (req->reqType == PROTO_REQUEST_REMOVEALIAS) {
         req->reqData.removeAlias.subject.type = SUBJECT_TYPE_ANY;
         req->reqData.removeAlias.subject.name = NULL;
      } else if (req->reqType == PROTO_REQUEST_CREATETICKET) {
         req->reqData.createTicket.samlData.aliasInfo.type = SUBJECT_TYPE_ANY;
         req->reqData.createTicket.samlData.aliasInfo.name = NULL;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found ANYSUBJECT in req type %d",
                     req->reqType);
         goto done;
       }
      break;
   case PARSE_STATE_COMMENT:
      if (req->reqType == PROTO_REQUEST_ADDALIAS) {
         SET_CHECK_DUP(req->reqData.addAlias.aliasInfo.comment, val);
      } else if (req->reqType == PROTO_REQUEST_CREATETICKET) {
         SET_CHECK_DUP(req->reqData.createTicket.samlData.aliasInfo.comment, val);
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found comment in req type %d",
                     req->reqType);
         goto done;
      }
      val = NULL;
      break;
   case PARSE_STATE_PEMCERT:
      if (req->reqType == PROTO_REQUEST_ADDALIAS) {
         SET_CHECK_DUP(req->reqData.addAlias.pemCert, val);
      } else if (req->reqType == PROTO_REQUEST_REMOVEALIAS) {
         SET_CHECK_DUP(req->reqData.removeAlias.pemCert, val);
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found pemCert in req type %d",
                     req->reqType);
         goto done;
      }
      val = NULL;
      break;
   case PARSE_STATE_PID:
      switch (req->reqType) {
      case PROTO_REQUEST_CONN:
         SET_CHECK_DUP(req->reqData.connect.pid, val);
         break;
      default:
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found pid in req type %d",
                     req->reqType);
         goto done;
      }
      val = NULL;
      break;
   case PARSE_STATE_TOKEN:
      switch (req->reqType) {
      case PROTO_REQUEST_CREATETICKET:
         SET_CHECK_DUP(req->reqData.createTicket.token, val);
         break;
      default:
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found token in req type %d",
                     req->reqType);
         goto done;
      }
      val = NULL;
      break;
   case PARSE_STATE_SAMLTOKEN:
      if (req->reqType != PROTO_REQUEST_VALIDATE_SAML_BEARER_TOKEN) {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found SAML token in req type %d",
                     req->reqType);
         goto done;
      }
      SET_CHECK_DUP(req->reqData.validateSamlBToken.samlToken, val);
      val = NULL;
      break;
   case PARSE_STATE_SAML_VALIDATE_ONLY:

      if (req->reqType != PROTO_REQUEST_VALIDATE_SAML_BEARER_TOKEN) {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found validateOnly option in req type %d",
                     req->reqType);
         goto done;
      }
      iVal = atoi(val);
      req->reqData.validateSamlBToken.validateOnly = (iVal) ? TRUE : FALSE;
      break;
   case PARSE_STATE_USERHANDLETYPE:
      {
      ServiceValidationResultsType t = VALIDATION_RESULTS_TYPE_UNKNOWN;

      if (req->reqType != PROTO_REQUEST_CREATETICKET) {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found userHandle type in req type %d",
                     req->reqType);
         goto done;
      }
      if (g_strcmp0(val, VGAUTH_USERHANDLE_TYPE_NAMEPASSWORD) == 0) {
         t = VALIDATION_RESULTS_TYPE_NAMEPASSWORD;
      } else if (g_strcmp0(val, VGAUTH_USERHANDLE_TYPE_SSPI) == 0) {
         t = VALIDATION_RESULTS_TYPE_SSPI;
      } else if (g_strcmp0(val, VGAUTH_USERHANDLE_TYPE_SAML) == 0) {
         t = VALIDATION_RESULTS_TYPE_SAML;
      } else if (g_strcmp0(val, VGAUTH_USERHANDLE_TYPE_SAML_INFO_ONLY) == 0) {
         t = VALIDATION_RESULTS_TYPE_SAML_INFO_ONLY;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found unrecognized userHandle type %s", val);
         goto done;
      }
      req->reqData.createTicket.type = t;
      // let val be freed below
      }
      break;
   case PARSE_STATE_USERHANDLESAMLSUBJECT:
      if (req->reqType != PROTO_REQUEST_CREATETICKET) {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Found samlSubject in req type %d",
                     req->reqType);
         goto done;
      }
      SET_CHECK_DUP(req->reqData.createTicket.samlData.samlSubject, val);
      val = NULL;
      break;
   default:
      ASSERT(0);
      break;
   }
done:
   if (duplicate_found) {
      g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                  "Unexpected contents '%s' in parse state %d",
                  val, req->parseState);
   }
   g_free(val);
#undef SET_CHECK_DUP
}