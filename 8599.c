void Curl_conncache_close_all_connections(struct conncache *connc)
{
  struct connectdata *conn;
  char buffer[READBUFFER_MIN + 1];
  SIGPIPE_VARIABLE(pipe_st);
  if(!connc->closure_handle)
    return;
  connc->closure_handle->state.buffer = buffer;
  connc->closure_handle->set.buffer_size = READBUFFER_MIN;

  conn = conncache_find_first_connection(connc);
  while(conn) {
    sigpipe_ignore(connc->closure_handle, &pipe_st);
    /* This will remove the connection from the cache */
    connclose(conn, "kill all");
    Curl_conncache_remove_conn(connc->closure_handle, conn, TRUE);
    Curl_disconnect(connc->closure_handle, conn, FALSE);
    sigpipe_restore(&pipe_st);

    conn = conncache_find_first_connection(connc);
  }

  connc->closure_handle->state.buffer = NULL;
  sigpipe_ignore(connc->closure_handle, &pipe_st);

  Curl_hostcache_clean(connc->closure_handle,
                       connc->closure_handle->dns.hostcache);
  Curl_close(&connc->closure_handle);
  sigpipe_restore(&pipe_st);
}