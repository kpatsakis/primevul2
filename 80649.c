static bool VerifyNumber(const uint8* buffer,
                         int buffer_size,
                         int* offset,
                         int max_digits) {
  RCHECK(*offset < buffer_size);

  while (isspace(buffer[*offset])) {
    ++(*offset);
    RCHECK(*offset < buffer_size);
  }

  int numSeen = 0;
  while (--max_digits >= 0 && isdigit(buffer[*offset])) {
    ++numSeen;
    ++(*offset);
    if (*offset >= buffer_size)
      return true;  // Out of space but seen a digit.
  }

  return (numSeen > 0);
}
