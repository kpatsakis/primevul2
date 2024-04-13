Proto_DumpRequest(ProtoRequest *req)
{
#if VGAUTH_PROTO_TRACE
   printf("raw data: %s\n", req->rawData ? req->rawData : "<none>");
#endif
   Debug("complete: %d\n", req->complete);
   Debug("sequenceNumber: %d\n", req->sequenceNumber);
   Log("requestType: %d(%s REQ)\n", req->reqType,
       ProtoRequestTypeText(req->reqType));

   switch (req->reqType) {
   case PROTO_REQUEST_SESSION_REQ:
      Debug("version #: %d\n", req->reqData.sessionReq.version);
      Log("userName: '%s'\n", req->reqData.sessionReq.userName);
      break;
   case PROTO_REQUEST_CONN:
      // no details
      break;
   case PROTO_REQUEST_ADDALIAS:
      Log("userName: %s\n", req->reqData.addAlias.userName);
      Log("addMapped: %d\n", req->reqData.addAlias.addMapped);
      Debug("pemCert: %s\n", req->reqData.addAlias.pemCert);
      if (req->reqData.addAlias.aliasInfo.type == SUBJECT_TYPE_NAMED) {
         Log("Subject: %s\n", req->reqData.addAlias.aliasInfo.name);
      } else  if (req->reqData.addAlias.aliasInfo.type == SUBJECT_TYPE_ANY) {
         Log("ANY Subject\n");
      } else {
         Warning("*** UNKNOWN Subject type ***\n");
      }
      Log("comment: %s\n", req->reqData.addAlias.aliasInfo.comment);
      break;
   case PROTO_REQUEST_REMOVEALIAS:
      Log("userName: %s\n", req->reqData.removeAlias.userName);
      Debug("pemCert: %s\n", req->reqData.removeAlias.pemCert);
      if (req->reqData.removeAlias.subject.type == SUBJECT_TYPE_NAMED) {
         Log("Subject: %s\n", req->reqData.removeAlias.subject.name);
      } else  if (req->reqData.removeAlias.subject.type == SUBJECT_TYPE_ANY) {
         Log("ANY Subject\n");
      } else {
         Log("No Subject type specified (assuming removeAll case)\n");
      }
      break;
   case PROTO_REQUEST_QUERYALIASES:
      Log("userName: %s\n", req->reqData.queryAliases.userName);
      break;
   case PROTO_REQUEST_QUERYMAPPEDALIASES:
      // no details
      break;
   case PROTO_REQUEST_CREATETICKET:
      Log("userName '%s'\n", req->reqData.createTicket.userName);
      break;
   case PROTO_REQUEST_VALIDATETICKET:
      Log("ticket '%s'\n", req->reqData.validateTicket.ticket);
      break;
   case PROTO_REQUEST_REVOKETICKET:
      Log("ticket '%s'\n", req->reqData.revokeTicket.ticket);
      break;
   case PROTO_REQUEST_VALIDATE_SAML_BEARER_TOKEN:
      Debug("token '%s'\n", req->reqData.validateSamlBToken.samlToken);
      Log("username '%s'\n", req->reqData.validateSamlBToken.userName);
      Log("validate Only '%s'\n",
            req->reqData.validateSamlBToken.validateOnly ? "TRUE" : "FALSE");
      break;
   default:
      Warning("Unknown request type -- no request specific data\n");
      break;
   }
}