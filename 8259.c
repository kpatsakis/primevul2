static CURLUcode decode_host(char *hostname, char **outp)
{
  char *per = NULL;
  if(hostname[0] != '[')
    /* only decode if not an ipv6 numerical */
    per = strchr(hostname, '%');
  if(!per) {
    *outp = strdup(hostname);
    if(!*outp)
      return CURLUE_OUT_OF_MEMORY;
  }
  else {
    /* might be encoded */
    size_t dlen;
    CURLcode result = Curl_urldecode(hostname, 0, outp, &dlen, REJECT_CTRL);
    if(result)
      return CURLUE_BAD_HOSTNAME;
  }

  return CURLUE_OK;
}