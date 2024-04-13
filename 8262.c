bool Curl_is_absolute_url(const char *url, char *buf, size_t buflen)
{
  int i;
  DEBUGASSERT(!buf || (buflen > MAX_SCHEME_LEN));
  (void)buflen; /* only used in debug-builds */
  if(buf)
    buf[0] = 0; /* always leave a defined value in buf */
#ifdef WIN32
  if(STARTS_WITH_DRIVE_PREFIX(url))
    return FALSE;
#endif
  for(i = 0; i < MAX_SCHEME_LEN; ++i) {
    char s = url[i];
    if(s && (ISALNUM(s) || (s == '+') || (s == '-') || (s == '.') )) {
      /* RFC 3986 3.1 explains:
        scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
      */
    }
    else {
      break;
    }
  }
  if(i && (url[i] == ':') && (url[i + 1] == '/')) {
    if(buf) {
      buf[i] = 0;
      while(i--) {
        buf[i] = (char)TOLOWER(url[i]);
      }
    }
    return TRUE;
  }
  return FALSE;
}