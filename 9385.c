murmur_hash64(const void *key, int len, uint32_t seed)
{
  const int r = 47;
  const uint64_t m = 0xc6a4a7935bd1e995;

  const uint64_t *data;
  const uint64_t *end;
  const unsigned char *data_tail;
  uint64_t h;
  uint64_t k;

  h = seed ^ (len * m);
  data = (const uint64_t *)key;
  end = data + (len / 8);

  while (data != end)
    {
      k = *data++;

      k *= m;
      k ^= k >> r;
      k *= m;

      h ^= k;
      h *= m;
    }

  data_tail = (const unsigned char *)data;

  switch (len & 7)
    {
      case 7:
	h ^= (uint64_t)(data_tail[6]) << 48; /* FALLTHROUGH */
      case 6:
	h ^= (uint64_t)(data_tail[5]) << 40; /* FALLTHROUGH */
      case 5:
	h ^= (uint64_t)(data_tail[4]) << 32; /* FALLTHROUGH */
      case 4:
	h ^= (uint64_t)(data_tail[3]) << 24; /* FALLTHROUGH */
      case 3:
	h ^= (uint64_t)(data_tail[2]) << 16; /* FALLTHROUGH */
      case 2:
	h ^= (uint64_t)(data_tail[1]) << 8; /* FALLTHROUGH */
      case 1:
	h ^= (uint64_t)(data_tail[0]);
	h *= m;
    }

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}