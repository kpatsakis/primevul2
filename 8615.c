Proto_StartElement(GMarkupParseContext *parseContext,
                   const gchar *elementName,
                   const gchar **attributeNames,
                   const gchar **attributeValues,
                   gpointer userData,
                   GError **error)
{
   ProtoRequest *req = (ProtoRequest *) userData;

#if VGAUTH_PROTO_TRACE
   Debug("%s: elementName '%s', parseState %d, request type %d\n", __FUNCTION__, elementName, req->parseState, req->reqType);
#endif

   switch (req->parseState) {
   case PARSE_STATE_NONE:
      /*
       * We're in 'idle' mode, expecting a fresh request.
       */
      if (g_strcmp0(elementName, VGAUTH_REQUEST_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_REQUEST;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Unexpected element '%s' in parse state %d",
                     elementName, req->parseState);
      }
      break;
   case PARSE_STATE_REQUEST:
      /*
       * We're in 'request' mode, expecting some element inside the request.
       */
      if (g_strcmp0(elementName, VGAUTH_REQUESTNAME_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_REQNAME;
      } else if (g_strcmp0(elementName, VGAUTH_SEQUENCENO_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_SEQNO;
      } else if (g_strcmp0(elementName, VGAUTH_USERNAME_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_USERNAME;
      } else if (g_strcmp0(elementName, VGAUTH_VERSION_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_VERSION;
      } else if (g_strcmp0(elementName, VGAUTH_TICKET_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_TICKET;
      } else if (g_strcmp0(elementName, VGAUTH_ADDMAPPEDLINK_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_ADDLINK;
      } else if (g_strcmp0(elementName, VGAUTH_PEMCERT_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_PEMCERT;
      } else if (g_strcmp0(elementName, VGAUTH_PID_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_PID;
      } else if (g_strcmp0(elementName, VGAUTH_TOKEN_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_TOKEN;
      } else if (g_strcmp0(elementName, VGAUTH_SAMLTOKEN_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_SAMLTOKEN;
      } else if (g_strcmp0(elementName, VGAUTH_VALIDATE_ONLY_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_SAML_VALIDATE_ONLY;
      } else if (g_strcmp0(elementName, VGAUTH_ALIASINFO_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_ALIASINFO;
      } else if (g_strcmp0(elementName, VGAUTH_SUBJECT_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_SUBJECT;
      } else if (g_strcmp0(elementName, VGAUTH_USERHANDLEINFO_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_USERHANDLEINFO;
      } else if (g_strcmp0(elementName, VGAUTH_ANYSUBJECT_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_ANYSUBJECT;
         /*
          * Proto_TextContents will never get called for an empty-element
          * tag, so set the value here.
          */
         req->parseState = PARSE_STATE_ANYSUBJECT;
         if (req->reqType != PROTO_REQUEST_REMOVEALIAS) {
            g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                        "Unexpected element '%s' in parse state %d",
                        elementName, req->parseState);
         } else {
            req->reqData.removeAlias.subject.type = SUBJECT_TYPE_ANY;
         }
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Unexpected element '%s' in parse state %d",
                     elementName, req->parseState);
      }
      break;
   case PARSE_STATE_ALIASINFO:
      /*
       * We're parsing a AliasInfo, expecting one of its components.
       */
      if (g_strcmp0(elementName, VGAUTH_SUBJECT_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_SUBJECT;
      } else if (g_strcmp0(elementName, VGAUTH_ANYSUBJECT_ELEMENT_NAME) == 0) {
         /*
          * Proto_TextContents will never get called for an empty-element
          * tag, so set the value here.
          */
         req->parseState = PARSE_STATE_ANYSUBJECT;
         if (req->reqType == PROTO_REQUEST_ADDALIAS) {
            req->reqData.addAlias.aliasInfo.type = SUBJECT_TYPE_ANY;
         } else if (req->reqType == PROTO_REQUEST_CREATETICKET) {
            req->reqData.createTicket.samlData.aliasInfo.type = SUBJECT_TYPE_ANY;
         } else {
            g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                        "Unexpected element '%s' in parse state %d",
                        elementName, req->parseState);
         }
      } else if (g_strcmp0(elementName, VGAUTH_COMMENT_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_COMMENT;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Unexpected element '%s' in parse state %d",
                     elementName, req->parseState);
      }
      break;
   case PARSE_STATE_USERHANDLEINFO:
      /*
       * We're parsing a UserHandleInfo, expecting one of its components.
       */
      if (g_strcmp0(elementName, VGAUTH_USERHANDLETYPE_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_USERHANDLETYPE;
      } else if (g_strcmp0(elementName, VGAUTH_USERHANDLESAMLINFO_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_USERHANDLESAMLINFO;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Unexpected element '%s' in parse state %d",
                     elementName, req->parseState);
      }
      break;
   case PARSE_STATE_USERHANDLESAMLINFO:
      /*
       * We're parsing a UserHandleSamlInfo, expecting one of its components.
       */
      if (g_strcmp0(elementName, VGAUTH_USERHANDLESAMLSUBJECT_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_USERHANDLESAMLSUBJECT;
      } else if (g_strcmp0(elementName, VGAUTH_ALIASINFO_ELEMENT_NAME) == 0) {
         req->parseState = PARSE_STATE_ALIASINFO;
      } else {
         g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                     "Unexpected element '%s' in parse state %d",
                     elementName, req->parseState);
      }
      break;
   default:
      g_set_error(error, G_MARKUP_ERROR_PARSE, VGAUTH_E_INVALID_ARGUMENT,
                  "Unexpected element '%s' in parse state %d",
                  elementName, req->parseState);
      break;
   }
}