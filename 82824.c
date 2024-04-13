static bool IsValidNameNonASCII(const UChar* characters, unsigned length) {
  for (unsigned i = 0; i < length;) {
    bool first = i == 0;
    UChar32 c;
    U16_NEXT(characters, i, length, c);  // Increments i.
    if (first ? !IsValidNameStart(c) : !IsValidNamePart(c))
      return false;
  }

  return true;
}
