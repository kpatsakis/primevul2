bool Curl_conncache_return_conn(struct Curl_easy *data,
                                struct connectdata *conn)
{
  /* data->multi->maxconnects can be negative, deal with it. */
  size_t maxconnects =
    (data->multi->maxconnects < 0) ? data->multi->num_easy * 4:
    data->multi->maxconnects;
  struct connectdata *conn_candidate = NULL;

  conn->lastused = Curl_now(); /* it was used up until now */
  if(maxconnects > 0 &&
     Curl_conncache_size(data) > maxconnects) {
    infof(data, "Connection cache is full, closing the oldest one");

    conn_candidate = Curl_conncache_extract_oldest(data);
    if(conn_candidate) {
      /* the winner gets the honour of being disconnected */
      Curl_disconnect(data, conn_candidate, /* dead_connection */ FALSE);
    }
  }

  return (conn_candidate == conn) ? FALSE : TRUE;

}