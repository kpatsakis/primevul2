ServiceProtoRevokeTicket(ServiceConnection *conn,
                         ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;

   err = ServiceRevokeTicket(conn, req->reqData.revokeTicket.ticket);
   if (err != VGAUTH_E_OK) {
      packet = Proto_MakeErrorReply(conn, req, err, "revokeTicket failed");
   } else {
      packet = g_markup_printf_escaped(VGAUTH_REVOKETICKET_REPLY_FORMAT,
                                       req->sequenceNumber);
   }

   err = ServiceNetworkWriteData(conn, strlen(packet), packet);
   if (err != VGAUTH_E_OK) {
      VGAUTH_LOG_WARNING("ServiceNetWorkWriteData() failed, pipe = %s",
                         conn->pipeName);
   }

   g_free(packet);

   return err;
}