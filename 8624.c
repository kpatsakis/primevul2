ServiceProtoRemoveAlias(ServiceConnection *conn,
                        ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;

   /*
    * The alias code will do argument validation.
    */
   err = ServiceAliasRemoveAlias(conn->userName,
                                 req->reqData.removeAlias.userName,
                                 req->reqData.removeAlias.pemCert,
                                 &(req->reqData.removeAlias.subject));

   if (err != VGAUTH_E_OK) {
      packet = Proto_MakeErrorReply(conn, req, err, "removeAlias failed");
   } else {
      packet = g_markup_printf_escaped(VGAUTH_REMOVEALIAS_REPLY_FORMAT,
                                       req->sequenceNumber);
   }

   err = ServiceNetworkWriteData(conn, strlen(packet), packet);
   if (err != VGAUTH_E_OK) {
      Warning("%s: failed to send RemoveAlias reply\n", __FUNCTION__);
   }

   g_free(packet);

   return err;
}