trim(char *str)
{
  size_t start; // Position of first non-space char
  size_t term;  // Position of 0-terminator

  if (!str)
    return NULL;

  start = 0;
  term  = strlen(str);

  while ((start < term) && isspace(str[start]))
    start++;
  while ((term > start) && isspace(str[term - 1]))
    term--;

  str[term] = '\0';

  // Shift chars incl. terminator
  if (start)
    memmove(str, str + start, term - start + 1);

  return str;
}