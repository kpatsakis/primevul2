size_t Curl_conncache_size(struct Curl_easy *data)
{
  size_t num;
  CONNCACHE_LOCK(data);
  num = data->state.conn_cache->num_conn;
  CONNCACHE_UNLOCK(data);
  return num;
}