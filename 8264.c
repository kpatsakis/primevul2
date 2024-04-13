UNITTEST CURLUcode Curl_parse_port(struct Curl_URL *u, char *hostname,
                                   bool has_scheme)
{
  char *portptr = NULL;
  char endbracket;
  int len;

  /*
   * Find the end of an IPv6 address, either on the ']' ending bracket or
   * a percent-encoded zone index.
   */
  if(1 == sscanf(hostname, "[%*45[0123456789abcdefABCDEF:.]%c%n",
                 &endbracket, &len)) {
    if(']' == endbracket)
      portptr = &hostname[len];
    else if('%' == endbracket) {
      int zonelen = len;
      if(1 == sscanf(hostname + zonelen, "%*[^]]%c%n", &endbracket, &len)) {
        if(']' != endbracket)
          return CURLUE_BAD_IPV6;
        portptr = &hostname[--zonelen + len + 1];
      }
      else
        return CURLUE_BAD_IPV6;
    }
    else
      return CURLUE_BAD_IPV6;

    /* this is a RFC2732-style specified IP-address */
    if(portptr && *portptr) {
      if(*portptr != ':')
        return CURLUE_BAD_IPV6;
    }
    else
      portptr = NULL;
  }
  else
    portptr = strchr(hostname, ':');

  if(portptr) {
    char *rest;
    long port;
    char portbuf[7];

    /* Browser behavior adaptation. If there's a colon with no digits after,
       just cut off the name there which makes us ignore the colon and just
       use the default port. Firefox, Chrome and Safari all do that.

       Don't do it if the URL has no scheme, to make something that looks like
       a scheme not work!
    */
    if(!portptr[1]) {
      *portptr = '\0';
      return has_scheme ? CURLUE_OK : CURLUE_BAD_PORT_NUMBER;
    }

    if(!ISDIGIT(portptr[1]))
      return CURLUE_BAD_PORT_NUMBER;

    port = strtol(portptr + 1, &rest, 10);  /* Port number must be decimal */

    if(port > 0xffff)
      return CURLUE_BAD_PORT_NUMBER;

    if(rest[0])
      return CURLUE_BAD_PORT_NUMBER;

    *portptr++ = '\0'; /* cut off the name there */
    *rest = 0;
    /* generate a new port number string to get rid of leading zeroes etc */
    msnprintf(portbuf, sizeof(portbuf), "%ld", port);
    u->portnum = port;
    u->port = strdup(portbuf);
    if(!u->port)
      return CURLUE_OUT_OF_MEMORY;
  }

  return CURLUE_OK;
}