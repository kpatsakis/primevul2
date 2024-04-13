static CURLcode bundle_create(struct connectbundle **bundlep)
{
  DEBUGASSERT(*bundlep == NULL);
  *bundlep = malloc(sizeof(struct connectbundle));
  if(!*bundlep)
    return CURLE_OUT_OF_MEMORY;

  (*bundlep)->num_connections = 0;
  (*bundlep)->multiuse = BUNDLE_UNKNOWN;

  Curl_llist_init(&(*bundlep)->conn_list, (Curl_llist_dtor) conn_llist_dtor);
  return CURLE_OK;
}