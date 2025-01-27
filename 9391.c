safe_snreplace(char *s, size_t sz, const char *pattern, const char *replacement)
{
  char *ptr;
  char *src;
  char *dst;
  size_t num;

  if (!s)
    return -1;

  if (!pattern || !replacement)
    return 0;

  size_t p_len = strlen(pattern);
  size_t r_len = strlen(replacement);
  size_t s_len = strlen(s) + 1; // Incl terminator

  ptr = s;
  while ((ptr = strstr(ptr, pattern)))
    {
      // We will move the part of the string after the pattern from src to dst
      src = ptr + p_len;
      dst = ptr + r_len;

      num = s_len - (src - s); // Number of bytes w/terminator we need to move
      if (dst + num > s + sz)
	return -1; // Not enough room

      // Shift everything after the pattern to the right, use memmove since
      // there might be an overlap
      memmove(dst, src, num);

      // Write replacement, no null terminater
      memcpy(ptr, replacement, r_len);

      // Advance ptr to avoid infinite looping
      ptr = dst;
    }

  return 0;
}