ProtoRequestTypeText(ProtoRequestType t)
{
   switch (t) {
   case PROTO_REQUEST_UNKNOWN:
      return "UNKNOWN";
   case PROTO_REQUEST_SESSION_REQ:
      return "SESSION";
   case PROTO_REQUEST_CONN:
      return "CONNECT";
   case PROTO_REQUEST_ADDALIAS:
      return "ADDALIAS";
   case PROTO_REQUEST_REMOVEALIAS:
      return "REMOVEALIAS";
   case PROTO_REQUEST_QUERYALIASES:
      return "QUERYALIASES";
   case PROTO_REQUEST_QUERYMAPPEDALIASES:
      return "QUERYMAPPEDALIASES";
   case PROTO_REQUEST_CREATETICKET:
      return "CREATETICKET";
   case PROTO_REQUEST_VALIDATETICKET:
      return "VALIDATETICKET";
   case PROTO_REQUEST_REVOKETICKET:
      return "REVOKETICKET";
   case PROTO_REQUEST_VALIDATE_SAML_BEARER_TOKEN:
      return "VALIDATE_SAML_BEARER_TOKEN";
   default:
      return "INVALID";
   }
}