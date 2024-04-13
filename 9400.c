two_str_hash(const char *a, const char *b)
{
  char hashbuf[2048];
  int64_t hash;
  int i;
  int ret;

  ret = snprintf(hashbuf, sizeof(hashbuf), "%s==%s", (a) ? a : "", (b) ? b : "");
  if (ret < 0 || ret == sizeof(hashbuf))
    {
      DPRINTF(E_LOG, L_MISC, "Buffer too large to calculate hash: '%s==%s'\n", a, b);
      return 999999; // Stand-in hash...
    }

  for (i = 0; hashbuf[i]; i++)
    hashbuf[i] = tolower(hashbuf[i]);

  // Limit hash length to 63 bits, due to signed type in sqlite
  hash = murmur_hash64(hashbuf, strlen(hashbuf), 0) >> 1;

  return hash;
}