ServiceProtoValidateTicket(ServiceConnection *conn,
                           ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;
   gchar *sPacket;
   char *userName = NULL;
   char *token = NULL;
   ServiceValidationResultsType type;
   ServiceValidationResultsData *svd = NULL;

   /*
    * The ticket code will do argument validation.
    */
#ifdef _WIN32
   err = ServiceValidateTicketWin(req->reqData.validateTicket.ticket,
                                  conn->hProc,
                                  &userName,
                                  &type,
                                  &svd,
                                  &token);
#else
   err = ServiceValidateTicketPosix(req->reqData.validateTicket.ticket,
                                    &userName,
                                    &type,
                                    &svd);
#endif

   if (err != VGAUTH_E_OK) {
      packet = Proto_MakeErrorReply(conn, req, err, "validateTicket failed");
   } else {
      /* Value of token is always NULL on non-Windows platforms */
      /* coverity[dead_error_line] */
      packet = g_markup_printf_escaped(VGAUTH_VALIDATETICKET_REPLY_FORMAT_START,
                                       req->sequenceNumber,
                                       userName,
                                       token ? token : "",
                                       ProtoValidationTypeString(type));
      if (VALIDATION_RESULTS_TYPE_SAML == type) {
         sPacket = g_markup_printf_escaped(VGAUTH_USERHANDLESAMLINFO_FORMAT_START,
                                           svd->samlSubject);
         packet = Proto_ConcatXMLStrings(packet, sPacket);
         if (SUBJECT_TYPE_NAMED == svd->aliasInfo.type) {
               sPacket = g_markup_printf_escaped(VGAUTH_NAMEDALIASINFO_FORMAT,
                                                  svd->aliasInfo.name,
                                                  svd->aliasInfo.comment);
         } else {
               sPacket = g_markup_printf_escaped(VGAUTH_ANYALIASINFO_FORMAT,
                                                  svd->aliasInfo.comment);
         }
         packet = Proto_ConcatXMLStrings(packet, sPacket);
         packet = Proto_ConcatXMLStrings(packet,
                                         g_strdup(VGAUTH_USERHANDLESAMLINFO_FORMAT_END));
      }
      packet = Proto_ConcatXMLStrings(packet, g_strdup(VGAUTH_VALIDATETICKET_REPLY_FORMAT_END));
   }

   err = ServiceNetworkWriteData(conn, strlen(packet), packet);
   if (err != VGAUTH_E_OK) {
      VGAUTH_LOG_WARNING("ServiceNetWorkWriteData() failed, pipe = %s",
                         conn->pipeName);
      goto done;
   }

done:

   g_free(userName);
   g_free(token);
   g_free(packet);
   ServiceFreeValidationResultsData(svd);

   return err;
}