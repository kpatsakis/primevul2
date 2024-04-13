Proto_SanityCheckRequest(ProtoRequest *request)
{
   /*
    * There's not much else to do here for now.  The parser does
    * most of the work, and we have no other rules.  The service doesn't
    * care about sequence numbers, or matching a request to a reply.
    */
#if VGAUTH_PROTO_TRACE
   ASSERT(strncmp(request->rawData, VGAUTH_XML_PREAMBLE,
                  strlen(VGAUTH_XML_PREAMBLE)) == 0);
#endif
   return VGAUTH_E_OK;
}