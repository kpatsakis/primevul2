int Curl_conncache_init(struct conncache *connc, int size)
{
  /* allocate a new easy handle to use when closing cached connections */
  connc->closure_handle = curl_easy_init();
  if(!connc->closure_handle)
    return 1; /* bad */

  Curl_hash_init(&connc->hash, size, Curl_hash_str,
                 Curl_str_key_compare, free_bundle_hash_entry);
  connc->closure_handle->state.conn_cache = connc;

  return 0; /* good */
}