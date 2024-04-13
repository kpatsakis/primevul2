static void *conncache_add_bundle(struct conncache *connc,
                                  char *key,
                                  struct connectbundle *bundle)
{
  return Curl_hash_add(&connc->hash, key, strlen(key), bundle);
}