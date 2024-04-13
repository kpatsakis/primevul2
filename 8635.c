ServiceProtoCreateTicket(ServiceConnection *conn,
                         ProtoRequest *req)
{
   VGAuthError err;
   gchar *packet;
   char *ticket;

   /*
    * The ticket code will do argument validation.
    */
#ifdef _WIN32
   err = ServiceCreateTicketWin(req->reqData.createTicket.userName,
                                req->reqData.createTicket.type,
                                &(req->reqData.createTicket.samlData),
                                conn->hProc,
                                req->reqData.createTicket.token,
                                &ticket);
#else
   err = ServiceCreateTicketPosix(req->reqData.createTicket.userName,
                                  req->reqData.createTicket.type,
                                  &(req->reqData.createTicket.samlData),
                                  &ticket);
#endif

   if (err != VGAUTH_E_OK) {
      packet = Proto_MakeErrorReply(conn, req, err, "createTicket failed");
   } else {
      packet = g_markup_printf_escaped(VGAUTH_CREATETICKET_REPLY_FORMAT,
                                       req->sequenceNumber,
                                       ticket);
      g_free(ticket);
   }

   err = ServiceNetworkWriteData(conn, strlen(packet), packet);
   if (err != VGAUTH_E_OK) {
      Warning("%s: failed to send CreateTicket reply\n", __FUNCTION__);
   }

   g_free(packet);

   return err;
}