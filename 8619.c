ServiceProtoReadAndProcessRequest(ServiceConnection *conn)
{
   ProtoRequest *req = conn->curRequest;
   VGAuthError err = VGAUTH_E_OK;
   gchar *data = NULL;
   gsize len;
   gboolean bRet;
   GError *gErr = NULL;

   /*
    * If nothing is currently being processed start a fresh one.
    */
   if (NULL == req) {
      req = Proto_NewRequest();
      conn->curRequest = req;

      conn->parseContext = g_markup_parse_context_new(&wireParser,
                                                      0,
                                                      req,
                                                      NULL);
   }

   /*
    * Suck some bytes, parse them.
    */
   if (!req->complete) {
      err = ServiceNetworkReadData(conn, &len, &data);

      if (conn->eof) { // EOF
         err = VGAUTH_E_COMM;
         Debug("%s: read EOF on Connection %d\n", __FUNCTION__, conn->connId);
         goto quit;
      }

      if (err != VGAUTH_E_OK) {
         goto quit;
      }
#if VGAUTH_PROTO_TRACE
      if (req->rawData) {
         req->rawData = g_strdup_printf("%s%s", req->rawData, data);
      } else {
         req->rawData = g_strdup(data);
      }
#endif
      bRet = g_markup_parse_context_parse(conn->parseContext,
                                          data,
                                          len,
                                          &gErr);
      g_free(data);
      if (!bRet) {
         err = VGAUTH_E_COMM;
         Warning("%s: g_markup_parse_context_parse() failed: %s\n",
                 __FUNCTION__, gErr->message);
         g_error_free(gErr);
         goto quit;
      }
   }

   /*
    * If the parser says we have a complete request, process it.
    */
   if (req->complete) {
      Proto_DumpRequest(req);
      err = Proto_SanityCheckRequest(req);
      if (err != VGAUTH_E_OK) {
         Warning("%s: request sanity check failed\n", __FUNCTION__);
      }

      // only try to handle it if the sanity check passed
      if (err == VGAUTH_E_OK) {
         err = ServiceProtoDispatchRequest(conn, req);
      }

      /*
       * Reset the protocol parser.
       */
      ServiceProtoCleanupParseState(conn);
   }

quit:
   /*
    * If something went wrong, clean up.  Any error means bad data coming
    * from the client, and we don't even try to recover -- just slam
    * the door.
    */
   if (err != VGAUTH_E_OK) {
      ServiceConnectionShutdown(conn);
   }

   return err;
}