ServiceProtoQueryMappedAliases(ServiceConnection *conn,
                               ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;
   int num;
   ServiceMappedAlias *maList;

   /*
    * The alias code will do argument validation.
    */
   err = ServiceAliasQueryMappedAliases(&num,
                                        &maList);

   if (err != VGAUTH_E_OK) {
      packet = Proto_MakeErrorReply(conn, req, err, "queryMappedIds failed");
   } else {
      int i;
      gchar *endPacket;

      packet = g_markup_printf_escaped(VGAUTH_QUERYMAPPEDALIASES_REPLY_FORMAT_START,
                                       req->sequenceNumber);
      for (i = 0; i < num; i++) {
         gchar *tPacket;
         int j;

         tPacket = g_markup_printf_escaped(VGAUTH_MAPPEDALIASES_FORMAT_START,
                                           maList[i].userName,
                                           maList[i].pemCert);
         packet = Proto_ConcatXMLStrings(packet, tPacket);
         for (j = 0; j < maList[i].num; j++) {
            if (maList[i].subjects[j].type == SUBJECT_TYPE_ANY) {
               tPacket = g_markup_printf_escaped(VGAUTH_ANYSUBJECT_FORMAT);
            } else if (maList[i].subjects[j].type == SUBJECT_TYPE_NAMED) {
               tPacket = g_markup_printf_escaped(VGAUTH_SUBJECT_FORMAT,
                                                 maList[i].subjects[j].name);
            } else {
               tPacket = NULL;
               ASSERT(0);
            }
            packet = Proto_ConcatXMLStrings(packet, tPacket);
         }
         packet = Proto_ConcatXMLStrings(packet,
                                         g_markup_printf_escaped(VGAUTH_MAPPEDALIASES_FORMAT_END));
      }

      // now the end of the reply
      endPacket = g_markup_printf_escaped(VGAUTH_QUERYMAPPEDALIASES_REPLY_FORMAT_END);
      packet = Proto_ConcatXMLStrings(packet, endPacket);

      ServiceAliasFreeMappedAliasList(num, maList);
   }

   err = ServiceNetworkWriteData(conn, strlen(packet), packet);
   if (err != VGAUTH_E_OK) {
      Warning("%s: failed to send QueryAliases reply\n", __FUNCTION__);
   }

   g_free(packet);

   return err;
}