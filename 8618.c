Proto_NewRequest(void)
{
   ProtoRequest *req = g_malloc0(sizeof(ProtoRequest));

   req->parseState = PARSE_STATE_NONE;
   req->complete = FALSE;
#if VGAUTH_PROTO_TRACE
   req->rawData = NULL;
#endif
   req->reqType = PROTO_REQUEST_UNKNOWN;

   return req;
}