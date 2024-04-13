lexer_compare_identifier_to_chars (const uint8_t *left_p, /**< left identifier */
                                   const uint8_t *right_p, /**< right identifier string */
                                   size_t size) /**< byte size of the two identifiers */
{
  uint8_t utf8_buf[6];

  do
  {
    if (*left_p == *right_p)
    {
      left_p++;
      right_p++;
      size--;
      continue;
    }

    size_t escape_size;

    if (*left_p == LIT_CHAR_BACKSLASH)
    {
      left_p += 2;
      lit_code_point_t code_point = lexer_unchecked_hex_to_character (&left_p);

      escape_size = lit_code_point_to_cesu8_bytes (utf8_buf, code_point);
    }
    else if (*left_p >= LIT_UTF8_4_BYTE_MARKER)
    {
      lit_four_byte_utf8_char_to_cesu8 (utf8_buf, left_p);
      escape_size = 3 * 2;
      left_p += 4;
    }
    else
    {
      return false;
    }

    size -= escape_size;

    uint8_t *utf8_p = utf8_buf;
    do
    {
      if (*right_p++ != *utf8_p++)
      {
        return false;
      }
    } while (--escape_size > 0);
  } while (size > 0);

  return true;
} /* lexer_compare_identifier_to_chars */