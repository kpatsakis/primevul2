atrim(const char *str)
{
  size_t start; // Position of first non-space char
  size_t term;  // Position of 0-terminator
  size_t size;
  char *result;

  if (!str)
    return NULL;

  start = 0;
  term  = strlen(str);

  while ((start < term) && isspace(str[start]))
    start++;
  while ((term > start) && isspace(str[term - 1]))
    term--;

  size = term - start + 1;

  result = malloc(size);

  memcpy(result, str + start, size);
  result[size - 1] = '\0';

  return result;
}