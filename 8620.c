Proto_FreeRequest(ProtoRequest *req)
{
   if (NULL == req) {
      return;
   }

#if VGAUTH_PROTO_TRACE
   g_free(req->rawData);
#endif

   switch (req->reqType) {
   case PROTO_REQUEST_UNKNOWN:
      // partial/empty request -- no-op
      break;
   case PROTO_REQUEST_SESSION_REQ:
      g_free(req->reqData.sessionReq.userName);
      break;
   case PROTO_REQUEST_CONN:
      g_free(req->reqData.connect.pid);
      break;
   case PROTO_REQUEST_ADDALIAS:
      g_free(req->reqData.addAlias.userName);
      g_free(req->reqData.addAlias.pemCert);
      // will be NULL if ANY, so should be safe
      g_free(req->reqData.addAlias.aliasInfo.name);
      g_free(req->reqData.addAlias.aliasInfo.comment);
      break;
   case PROTO_REQUEST_REMOVEALIAS:
      g_free(req->reqData.removeAlias.userName);
      g_free(req->reqData.removeAlias.pemCert);
      // wll be NULL if ANY or unset, so should be safe
      g_free(req->reqData.removeAlias.subject.name);
      break;
   case PROTO_REQUEST_QUERYALIASES:
      g_free(req->reqData.queryAliases.userName);
      break;
   case PROTO_REQUEST_QUERYMAPPEDALIASES:
      //empty
      break;
   case PROTO_REQUEST_CREATETICKET:
      g_free(req->reqData.createTicket.userName);
      g_free(req->reqData.createTicket.token);
      ServiceAliasFreeAliasInfoContents(&(req->reqData.createTicket.samlData.aliasInfo));
      g_free(req->reqData.createTicket.samlData.samlSubject);
      break;
   case PROTO_REQUEST_VALIDATETICKET:
      g_free(req->reqData.validateTicket.ticket);
      break;
   case PROTO_REQUEST_REVOKETICKET:
      g_free(req->reqData.revokeTicket.ticket);
      break;
   case PROTO_REQUEST_VALIDATE_SAML_BEARER_TOKEN:
      g_free(req->reqData.validateSamlBToken.samlToken);
      g_free(req->reqData.validateSamlBToken.userName);
      break;
   default:
      Warning("%s: trying to free unknown request type %d\n",
              __FUNCTION__, req->reqType);
   }

   g_free(req);
}