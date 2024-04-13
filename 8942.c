lexer_hex_to_code_point (const uint8_t *source_p, /**< current source position */
                         parser_line_counter_t length) /**< source length */
{
  lit_code_point_t result = 0;

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
  } while (--length > 0);

  return result;
} /* lexer_hex_to_code_point */