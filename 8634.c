ServiceProtoDispatchRequest(ServiceConnection *conn,
                            ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;


   /*
    * Many requests must come across a superUser owned pipe.
    * Verify that here.
    */
   err = Proto_SecurityCheckRequest(conn, req);
   if (err != VGAUTH_E_OK) {
      Warning("%s: security check failed for request type %d\n",
              __FUNCTION__, req->reqType);
      packet = Proto_MakeErrorReply(conn,
                                    req,
                                    err,
                                    "Security check failed");
      goto sendError;
   }

#ifdef _WIN32
   /*
    * Check if we need to complete an earlier pid verification
    */
   if (conn->pidVerifyState == PID_VERIFY_PENDING) {
      err = ServiceEndVerifyPid(conn);
      if (err != VGAUTH_E_OK) {
         VGAUTH_LOG_WARNING("ServiceEndVerifyPid() failed, pipe = %s", conn->pipeName);
         packet = Proto_MakeErrorReply(conn,
                                       req,
                                       err,
                                       "Pid verification failed");
         goto sendError;
      }
   }

   /*
    * Check that we have the client proc handle to process the following
    * requests.
    */
   switch(req->reqType) {
   case PROTO_REQUEST_CREATETICKET:
   case PROTO_REQUEST_VALIDATETICKET:
   case PROTO_REQUEST_VALIDATE_SAML_BEARER_TOKEN:
      if (conn->hProc == NULL) {
         VGAUTH_LOG_WARNING("Invalid client process HANDLE, possibly missing Connect, "
                            "pipe = %s", conn->pipeName);
         err = VGAUTH_E_FAIL;
         packet = Proto_MakeErrorReply(conn,
                                       req,
                                       err,
                                       "Client process handle check failed");
         goto sendError;
      }

      break;
   default:
      break;
   }
#endif

   switch (req->reqType) {
   case PROTO_REQUEST_SESSION_REQ:
      err = ServiceProtoHandleSessionRequest(conn, req);
      break;
   case PROTO_REQUEST_CONN:
      err = ServiceProtoHandleConnection(conn, req);
      break;
   case PROTO_REQUEST_ADDALIAS:
      err = ServiceProtoAddAlias(conn, req);
      break;
   case PROTO_REQUEST_REMOVEALIAS:
      err = ServiceProtoRemoveAlias(conn, req);
      break;
   case PROTO_REQUEST_QUERYALIASES:
      err = ServiceProtoQueryAliases(conn, req);
      break;
   case PROTO_REQUEST_QUERYMAPPEDALIASES:
      err = ServiceProtoQueryMappedAliases(conn, req);
      break;
   case PROTO_REQUEST_CREATETICKET:
      err = ServiceProtoCreateTicket(conn, req);
      break;
   case PROTO_REQUEST_VALIDATETICKET:
      err = ServiceProtoValidateTicket(conn, req);
      break;
   case PROTO_REQUEST_REVOKETICKET:
      err = ServiceProtoRevokeTicket(conn, req);
      break;
   case PROTO_REQUEST_VALIDATE_SAML_BEARER_TOKEN:
      err = ServiceProtoValidateSamlBearerToken(conn, req);
      break;
   default:
      /*
       * Be polite, send an error, and then fail cleanly
       */
      err = VGAUTH_E_NOTIMPLEMENTED;
      packet = Proto_MakeErrorReply(conn,
                                    req,
                                    err,
                                    "Unrecognized request");
sendError:
      /*
       * Don't really care if it works since we're about to
       * shut it down anyways.
       */
      (void) ServiceNetworkWriteData(conn, strlen(packet), packet);
      g_free(packet);
      break;
   }

   // 'err' is from ServiceNetworkWriteData(), not from the operation
   Log("%s: processed reqType %d(%s REQ), returning "
       VGAUTHERR_FMT64" on connection %d\n", __FUNCTION__,
       req->reqType, ProtoRequestTypeText(req->reqType), err, conn->connId);

   return err;
}