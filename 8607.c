void Curl_conncache_destroy(struct conncache *connc)
{
  if(connc)
    Curl_hash_destroy(&connc->hash);
}