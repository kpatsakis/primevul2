ServiceProtoQueryAliases(ServiceConnection *conn,
                         ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;
   int num;
   ServiceAlias *aList;

   /*
    * The alias code will do argument validation.
    */
   err = ServiceAliasQueryAliases(req->reqData.queryAliases.userName,
                                  &num,
                                  &aList);

   if (err != VGAUTH_E_OK) {
      packet = Proto_MakeErrorReply(conn, req, err, "queryAliases failed");
   } else {
      int i;
      gchar *endPacket;

      packet = g_markup_printf_escaped(VGAUTH_QUERYALIASES_REPLY_FORMAT_START,
                                       req->sequenceNumber);
      // now the aliases
      for (i = 0; i < num; i++) {
         gchar *certPacket;
         int j;

         certPacket = g_markup_printf_escaped(VGAUTH_ALIAS_FORMAT_START,
                                              aList[i].pemCert);
         packet = Proto_ConcatXMLStrings(packet, certPacket);
         for (j = 0; j < aList[i].num; j++) {
            gchar *aiPacket;
            ServiceAliasInfo *ai = &(aList[i].infos[j]);

            if (ai->type == SUBJECT_TYPE_ANY) {
               aiPacket = g_markup_printf_escaped(VGAUTH_ANYALIASINFO_FORMAT,
                                                  ai->comment);
            } else if (ai->type == SUBJECT_TYPE_NAMED) {
               aiPacket = g_markup_printf_escaped(VGAUTH_NAMEDALIASINFO_FORMAT,
                                                  ai->name,
                                                  ai->comment);
            } else {
               aiPacket = NULL;
               ASSERT(0);
            }
            packet = Proto_ConcatXMLStrings(packet, aiPacket);
         }
         packet = Proto_ConcatXMLStrings(packet,
                                         g_markup_printf_escaped(VGAUTH_ALIAS_FORMAT_END));
      }

      // now the end of the reply
      endPacket = g_markup_printf_escaped(VGAUTH_QUERYALIASES_REPLY_FORMAT_END);
      packet = Proto_ConcatXMLStrings(packet, endPacket);


      ServiceAliasFreeAliasList(num, aList);
   }

   err = ServiceNetworkWriteData(conn, strlen(packet), packet);
   if (err != VGAUTH_E_OK) {
      Warning("%s: failed to send QueryAliases reply\n", __FUNCTION__);
   }

   g_free(packet);

   return err;
}