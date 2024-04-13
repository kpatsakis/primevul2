Curl_conncache_find_bundle(struct Curl_easy *data,
                           struct connectdata *conn,
                           struct conncache *connc,
                           const char **hostp)
{
  struct connectbundle *bundle = NULL;
  CONNCACHE_LOCK(data);
  if(connc) {
    char key[HASHKEY_SIZE];
    hashkey(conn, key, sizeof(key), hostp);
    bundle = Curl_hash_pick(&connc->hash, key, strlen(key));
  }

  return bundle;
}