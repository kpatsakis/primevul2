ServiceProtoHandleSessionRequest(ServiceConnection *conn,
                                 ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;
   gchar *pipeName = NULL;

   /*
    * Do any argument checking.  For now, the version number must
    * match.
    */

   if (req->reqData.sessionReq.version != atoi(VGAUTH_PROTOCOL_VERSION)) {
      err = VGAUTH_E_VERSION_MISMATCH;
      Warning("%s: version mismatch.  Client is %d, want %d\n",
              __FUNCTION__, req->reqData.sessionReq.version,
              atoi(VGAUTH_PROTOCOL_VERSION));
      packet = Proto_MakeErrorReply(conn, req, err,
                                    "sessionRequest failed; version mismatch");
      goto send_err;
   }

   err = ServiceStartUserConnection(req->reqData.sessionReq.userName,
                                    &pipeName);
   if (err != VGAUTH_E_OK) {
      packet = Proto_MakeErrorReply(conn, req, err, "sessionRequest failed");
   } else {
      packet = g_markup_printf_escaped(VGAUTH_SESSION_REPLY_FORMAT,
                                       req->sequenceNumber,
                                       pipeName);
   }

send_err:
   err = ServiceNetworkWriteData(conn, strlen(packet), packet);
   if (err != VGAUTH_E_OK) {
      Warning("%s: failed to send SessionReq reply\n", __FUNCTION__);
   }

   g_free(pipeName);
   g_free(packet);

   return err;
}