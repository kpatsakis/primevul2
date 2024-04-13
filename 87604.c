char CanonicalSchemeChar(base::char16 ch) {
  if (ch >= 0x80)
    return 0;  // Non-ASCII is not supported by schemes.
  return kSchemeCanonical[ch];
}
