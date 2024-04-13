ServiceProtoValidateSamlBearerToken(ServiceConnection *conn,
                                    ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;
   gchar *sPacket;
   char *userName = NULL;
   char *subjectName = NULL;
   char *comment = NULL;
   char *tokenStr = NULL;
   ServiceAliasInfo *ai = NULL;

   /*
    * The validate code will do argument validation.
    */
   err = SAML_VerifyBearerTokenAndChain(req->reqData.validateSamlBToken.samlToken,
                                        req->reqData.validateSamlBToken.userName,
                                        &userName,
                                        &subjectName,
                                        &ai);
#ifdef _WIN32
   /*
    * Only create a token in the non-info-only mode
    */
   if ((err == VGAUTH_E_OK) &&
       !req->reqData.validateSamlBToken.validateOnly) {
      HANDLE userToken = NULL;

      err = WinToken_GenerateTokenForUser(userName, &userToken);
      if (err == VGAUTH_E_OK) {
         tokenStr = ServiceDupHandleTo(conn->hProc, userToken);
         if (!tokenStr) {
            VGAUTH_LOG_WARNING("ServiceDupHandleTo() failed, user = %s",
                               userName);
            err = VGAUTH_E_FAIL;
         } else {
            // close our copy after duping into client process
            CloseHandle(userToken);
         }
      } else {
         VGAUTH_LOG_WARNING("WinToken_GenerateTokenForUser() failed, user = %s",
                            userName);
      }
   } else {
      Debug("%s: skipping token creation\n", __FUNCTION__);
   }
#endif
   if (err != VGAUTH_E_OK) {
      Audit_Event(FALSE,
                  SU_(validate.samlBearer.fail,
                      "Validation of SAML bearer token failed: %d"),
                  (int) err);    // localization code can't deal with
                                 // differing types of uint64


      /*
       * Rewrite some errors to hide any data that could be useful to an
       * attacker.  Do this at this stage so that we still have
       * useful debug and possibly auditing reasons.
       */
      if (err ==  VGAUTH_E_INVALID_CERTIFICATE) {
         err = VGAUTH_E_AUTHENTICATION_DENIED;
      }
      packet = Proto_MakeErrorReply(conn, req, err,
                                    "validateSamlToken failed");
   } else {
      Audit_Event(FALSE,
                  SU_(validate.samlBearer.success,
                      "Validated SAML bearer token for user '%s'"),
                  userName);
      /* Value of tokenStr is always NULL on non-Windows platforms */
      /* coverity[dead_error_line] */
      packet = g_markup_printf_escaped(VGAUTH_VALIDATESAMLBEARERTOKEN_REPLY_FORMAT_START,
                                       req->sequenceNumber,
                                       userName ? userName : "",
                                       tokenStr ? tokenStr : "",
                                       subjectName ? subjectName : "");

      if (SUBJECT_TYPE_NAMED == ai->type) {
            sPacket = g_markup_printf_escaped(VGAUTH_NAMEDALIASINFO_FORMAT,
                                               ai->name,
                                               ai->comment);
      } else {
            sPacket = g_markup_printf_escaped(VGAUTH_ANYALIASINFO_FORMAT,
                                              ai->comment);
      }
      packet = Proto_ConcatXMLStrings(packet, sPacket);
      packet = Proto_ConcatXMLStrings(packet,
                                      g_strdup(VGAUTH_VALIDATESAMLBEARERTOKEN_REPLY_FORMAT_END));
   }

   err = ServiceNetworkWriteData(conn, strlen(packet), packet);
   if (err != VGAUTH_E_OK) {
      VGAUTH_LOG_WARNING("ServiceNetWorkWriteData() failed, pipe = %s", conn->pipeName);
      goto done;
   }

done:
   g_free(userName);
   g_free(subjectName);
   g_free(packet);
   g_free(comment);
   g_free(tokenStr);
   ServiceAliasFreeAliasInfo(ai);

   return err;
}