struct Cookie *Curl_cookie_getlist(struct Curl_easy *data,
                                   struct CookieInfo *c,
                                   const char *host, const char *path,
                                   bool secure)
{
  struct Cookie *newco;
  struct Cookie *co;
  struct Cookie *mainco = NULL;
  size_t matches = 0;
  bool is_ip;
  const size_t myhash = cookiehash(host);

  if(!c || !c->cookies[myhash])
    return NULL; /* no cookie struct or no cookies in the struct */

  /* at first, remove expired cookies */
  remove_expired(c);

  /* check if host is an IP(v4|v6) address */
  is_ip = Curl_host_is_ipnum(host);

  co = c->cookies[myhash];

  while(co) {
    /* if the cookie requires we're secure we must only continue if we are! */
    if(co->secure?secure:TRUE) {

      /* now check if the domain is correct */
      if(!co->domain ||
         (co->tailmatch && !is_ip && tailmatch(co->domain, host)) ||
         ((!co->tailmatch || is_ip) && strcasecompare(host, co->domain)) ) {
        /*
         * the right part of the host matches the domain stuff in the
         * cookie data
         */

        /*
         * now check the left part of the path with the cookies path
         * requirement
         */
        if(!co->spath || pathmatch(co->spath, path) ) {

          /*
           * and now, we know this is a match and we should create an
           * entry for the return-linked-list
           */

          newco = dup_cookie(co);
          if(newco) {
            /* then modify our next */
            newco->next = mainco;

            /* point the main to us */
            mainco = newco;

            matches++;
            if(matches >= MAX_COOKIE_SEND_AMOUNT) {
              infof(data, "Included max number of cookies (%zu) in request!",
                    matches);
              break;
            }
          }
          else
            goto fail;
        }
      }
    }
    co = co->next;
  }

  if(matches) {
    /*
     * Now we need to make sure that if there is a name appearing more than
     * once, the longest specified path version comes first. To make this
     * the swiftest way, we just sort them all based on path length.
     */
    struct Cookie **array;
    size_t i;

    /* alloc an array and store all cookie pointers */
    array = malloc(sizeof(struct Cookie *) * matches);
    if(!array)
      goto fail;

    co = mainco;

    for(i = 0; co; co = co->next)
      array[i++] = co;

    /* now sort the cookie pointers in path length order */
    qsort(array, matches, sizeof(struct Cookie *), cookie_sort);

    /* remake the linked list order according to the new order */

    mainco = array[0]; /* start here */
    for(i = 0; i<matches-1; i++)
      array[i]->next = array[i + 1];
    array[matches-1]->next = NULL; /* terminate the list */

    free(array); /* remove the temporary data again */
  }

  return mainco; /* return the new list */

fail:
  /* failure, clear up the allocated chain and return NULL */
  Curl_cookie_freelist(mainco);
  return NULL;
}