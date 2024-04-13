ServiceReplyTooManyConnections(ServiceConnection *conn,
                               int connLimit)
{
   gchar *packet =
      ProtoMakeErrorReplyInt(conn, 0,
                             VGAUTH_E_TOO_MANY_CONNECTIONS,
                            "The user exceeded its max number of connections");

   (void) ServiceNetworkWriteData(conn, strlen(packet), packet);

   VGAUTH_LOG_WARNING("User %s exceeding concurrent connection limit of "
                      "%d connections (connection ID is %d)",
                      conn->userName, connLimit, conn->connId);

   g_free(packet);
}