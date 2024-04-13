lexer_hex_in_braces_to_code_point (const uint8_t *source_p, /**< current source position */
                                   const uint8_t *source_end_p, /**< source end */
                                   uint32_t *length_p) /**< [out] length of the sequence */
{
  lit_code_point_t result = 0;
  /* Four is the size of \u{} sequence. */
  uint32_t length = 4;

  JERRY_ASSERT (source_p[-1] == LIT_CHAR_LEFT_BRACE);
  JERRY_ASSERT (source_p < source_end_p);

  do
  {
    uint32_t byte = *source_p++;

    result <<= 4;

    if (byte >= LIT_CHAR_0 && byte <= LIT_CHAR_9)
    {
      result += byte - LIT_CHAR_0;
    }
    else
    {
      byte = LEXER_TO_ASCII_LOWERCASE (byte);
      if (byte >= LIT_CHAR_LOWERCASE_A && byte <= LIT_CHAR_LOWERCASE_F)
      {
        result += byte - (LIT_CHAR_LOWERCASE_A - 10);
      }
      else
      {
        return UINT32_MAX;
      }
    }

    if (result >= (LIT_UNICODE_CODE_POINT_MAX + 1) || source_p >= source_end_p)
    {
      return UINT32_MAX;
    }
    length++;
  } while (*source_p != LIT_CHAR_RIGHT_BRACE);

  *length_p = length;
  return result;
} /* lexer_hex_in_braces_to_code_point */