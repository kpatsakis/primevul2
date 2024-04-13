static CURLUcode parseurl(const char *url, CURLU *u, unsigned int flags)
{
  CURLUcode result = seturl(url, u, flags);
  if(result) {
    free_urlhandle(u);
    memset(u, 0, sizeof(struct Curl_URL));
  }
  return result;
}