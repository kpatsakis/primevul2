static size_t strlen_url(const char *url, bool relative)
{
  const unsigned char *ptr;
  size_t newlen = 0;
  bool left = TRUE; /* left side of the ? */
  const unsigned char *host_sep = (const unsigned char *) url;

  if(!relative)
    host_sep = (const unsigned char *) find_host_sep(url);

  for(ptr = (unsigned char *)url; *ptr; ptr++) {

    if(ptr < host_sep) {
      ++newlen;
      continue;
    }

    if(*ptr == ' ') {
      if(left)
        newlen += 3;
      else
        newlen++;
      continue;
    }

    if (*ptr == '?')
      left = FALSE;

    if(urlchar_needs_escaping(*ptr))
      newlen += 2;

    newlen++;
  }

  return newlen;
}