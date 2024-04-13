static CURLUcode parse_hostname_login(struct Curl_URL *u,
                                      char **hostname,
                                      unsigned int flags)
{
  CURLUcode result = CURLUE_OK;
  CURLcode ccode;
  char *userp = NULL;
  char *passwdp = NULL;
  char *optionsp = NULL;
  const struct Curl_handler *h = NULL;

  /* At this point, we're hoping all the other special cases have
   * been taken care of, so conn->host.name is at most
   *    [user[:password][;options]]@]hostname
   *
   * We need somewhere to put the embedded details, so do that first.
   */

  char *ptr = strchr(*hostname, '@');
  char *login = *hostname;

  if(!ptr)
    goto out;

  /* We will now try to extract the
   * possible login information in a string like:
   * ftp://user:password@ftp.my.site:8021/README */
  *hostname = ++ptr;

  /* if this is a known scheme, get some details */
  if(u->scheme)
    h = Curl_builtin_scheme(u->scheme);

  /* We could use the login information in the URL so extract it. Only parse
     options if the handler says we should. Note that 'h' might be NULL! */
  ccode = Curl_parse_login_details(login, ptr - login - 1,
                                   &userp, &passwdp,
                                   (h && (h->flags & PROTOPT_URLOPTIONS)) ?
                                   &optionsp:NULL);
  if(ccode) {
    result = CURLUE_BAD_LOGIN;
    goto out;
  }

  if(userp) {
    if(flags & CURLU_DISALLOW_USER) {
      /* Option DISALLOW_USER is set and url contains username. */
      result = CURLUE_USER_NOT_ALLOWED;
      goto out;
    }
    if(junkscan(userp, flags)) {
      result = CURLUE_BAD_USER;
      goto out;
    }
    u->user = userp;
  }

  if(passwdp) {
    if(junkscan(passwdp, flags)) {
      result = CURLUE_BAD_PASSWORD;
      goto out;
    }
    u->password = passwdp;
  }

  if(optionsp) {
    if(junkscan(optionsp, flags)) {
      result = CURLUE_BAD_LOGIN;
      goto out;
    }
    u->options = optionsp;
  }

  return CURLUE_OK;
  out:

  free(userp);
  free(passwdp);
  free(optionsp);
  u->user = NULL;
  u->password = NULL;
  u->options = NULL;

  return result;
}