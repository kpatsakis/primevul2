lexer_convert_ident_to_cesu8 (uint8_t *destination_p, /**< destination string */
                              const uint8_t *source_p, /**< source string */
                              prop_length_t length) /**< length of destination string */
{
  const uint8_t *destination_end_p = destination_p + length;

  JERRY_ASSERT (length <= PARSER_MAXIMUM_IDENT_LENGTH);

  do
  {
    if (*source_p == LIT_CHAR_BACKSLASH)
    {
      source_p += 2;
      destination_p += lit_code_point_to_cesu8_bytes (destination_p, lexer_unchecked_hex_to_character (&source_p));
      continue;
    }

#if JERRY_ESNEXT
    if (*source_p >= LIT_UTF8_4_BYTE_MARKER)
    {
      lit_four_byte_utf8_char_to_cesu8 (destination_p, source_p);

      destination_p += 6;
      source_p += 4;
      continue;
    }
#endif /* JERRY_ESNEXT */

    *destination_p++ = *source_p++;
  } while (destination_p < destination_end_p);
} /* lexer_convert_ident_to_cesu8 */