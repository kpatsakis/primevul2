static void hashkey(struct connectdata *conn, char *buf,
                    size_t len,  /* something like 128 is fine */
                    const char **hostp)
{
  const char *hostname;
  long port = conn->remote_port;

#ifndef CURL_DISABLE_PROXY
  if(conn->bits.httpproxy && !conn->bits.tunnel_proxy) {
    hostname = conn->http_proxy.host.name;
    port = conn->port;
  }
  else
#endif
    if(conn->bits.conn_to_host)
      hostname = conn->conn_to_host.name;
  else
    hostname = conn->host.name;

  if(hostp)
    /* report back which name we used */
    *hostp = hostname;

  /* put the numbers first so that the hostname gets cut off if too long */
#ifdef ENABLE_IPV6
  msnprintf(buf, len, "%u/%ld/%s", conn->scope_id, port, hostname);
#else
  msnprintf(buf, len, "%ld/%s", port, hostname);
#endif
  Curl_strntolower(buf, buf, len);
}