static void strcpy_url(char *output, const char *url, bool relative)
{
  /* we must add this with whitespace-replacing */
  bool left = TRUE;
  const unsigned char *iptr;
  char *optr = output;
  const unsigned char *host_sep = (const unsigned char *) url;

  if(!relative)
    host_sep = (const unsigned char *) find_host_sep(url);

  for(iptr = (unsigned char *)url;    /* read from here */
      *iptr;         /* until zero byte */
      iptr++) {

    if(iptr < host_sep) {
      *optr++ = *iptr;
      continue;
    }

    if(*iptr == ' ') {
      if(left) {
        *optr++='%'; /* add a '%' */
        *optr++='2'; /* add a '2' */
        *optr++='0'; /* add a '0' */
      }
      else
        *optr++='+'; /* add a '+' here */
      continue;
    }

    if(*iptr == '?')
      left = FALSE;

    if(urlchar_needs_escaping(*iptr)) {
      msnprintf(optr, 4, "%%%02x", *iptr);
      optr += 3;
    }
    else
      *optr++ = *iptr;
  }
  *optr = 0; /* null-terminate output buffer */

}