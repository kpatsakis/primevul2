ctcompare(const char *a,		/* I - First string */
          const char *b)		/* I - Second string */
{
  int	result = 0;			/* Result */


  while (*a && *b)
  {
    result |= *a ^ *b;
    a ++;
    b ++;
  }

  // either both *a and *b == '\0', or one points inside a string,
  // so factor that in.
  result |= (*a ^ *b);

  return (result);
}