ServiceProtoAddAlias(ServiceConnection *conn,
                     ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;

   /*
    * The alias code will do argument validation.
    */
   err = ServiceAliasAddAlias(conn->userName,
                              req->reqData.addAlias.userName,
                              req->reqData.addAlias.addMapped,
                              req->reqData.addAlias.pemCert,
                              &(req->reqData.addAlias.aliasInfo));

   if (err != VGAUTH_E_OK) {
      packet = Proto_MakeErrorReply(conn, req, err, "addAlias failed");
   } else {
      packet = g_markup_printf_escaped(VGAUTH_ADDALIAS_REPLY_FORMAT,
                                       req->sequenceNumber);
   }

   err = ServiceNetworkWriteData(conn, strlen(packet), packet);
   if (err != VGAUTH_E_OK) {
      Warning("%s: failed to send AddSubject reply\n", __FUNCTION__);
   }

   g_free(packet);

   return err;
}