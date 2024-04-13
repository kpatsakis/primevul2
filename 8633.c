Proto_MakeErrorReply(ServiceConnection *conn,
                     ProtoRequest *req,
                     VGAuthError err,
                     const char *errMsg)
{
   return ProtoMakeErrorReplyInt(conn, req->sequenceNumber, err, errMsg);
}