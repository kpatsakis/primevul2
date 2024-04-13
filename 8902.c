static int invalid_octets(const char *p)
{
  /* Reject all bytes \x01 - \x1f (*except* \x09, TAB) + \x7f */
  static const char badoctets[] = {
    "\x01\x02\x03\x04\x05\x06\x07\x08\x0a"
    "\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14"
    "\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f"
  };
  size_t vlen, len;
  /* scan for all the octets that are *not* in cookie-octet */
  len = strcspn(p, badoctets);
  vlen = strlen(p);
  return (len != vlen);
}