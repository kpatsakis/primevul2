lexer_construct_number_object (parser_context_t *context_p, /**< context */
                               bool is_expr, /**< expression is parsed */
                               bool is_negative_number) /**< sign is negative */
{
  parser_list_iterator_t literal_iterator;
  lexer_literal_t *literal_p;
  ecma_value_t lit_value;
  uint32_t literal_index = 0;
  prop_length_t length = context_p->token.lit_location.length;

#if JERRY_BUILTIN_BIGINT
  if (JERRY_LIKELY (context_p->token.extra_value != LEXER_NUMBER_BIGINT))
  {
#endif /* JERRY_BUILTIN_BIGINT */
    ecma_number_t num;
    uint32_t options = 0;

#if JERRY_ESNEXT
    options |= ECMA_CONVERSION_ALLOW_UNDERSCORE;
#endif /* JERRY_ESNEXT */

    if (context_p->token.extra_value == LEXER_NUMBER_OCTAL)
    {
      num = ecma_utf8_string_to_number_by_radix (context_p->token.lit_location.char_p, length, 8, options);
    }
    else
    {
      num = ecma_utf8_string_to_number (context_p->token.lit_location.char_p, length, options);
    }

    if (is_expr)
    {
      int32_t int_num = (int32_t) num;

      if (int_num == num && int_num <= CBC_PUSH_NUMBER_BYTE_RANGE_END && (int_num != 0 || !is_negative_number))
      {
        context_p->lit_object.index = (uint16_t) int_num;
        return true;
      }
    }

    if (is_negative_number)
    {
      num = -num;
    }

    lit_value = ecma_find_or_create_literal_number (num);
#if JERRY_BUILTIN_BIGINT
  }
  else
  {
    uint32_t options = (ECMA_BIGINT_PARSE_DISALLOW_SYNTAX_ERROR | ECMA_BIGINT_PARSE_DISALLOW_MEMORY_ERROR
                        | ECMA_BIGINT_PARSE_ALLOW_UNDERSCORE);

    if (is_negative_number)
    {
      options |= ECMA_BIGINT_PARSE_SET_NEGATIVE;
    }

    JERRY_ASSERT (length >= 2);
    lit_value =
      ecma_bigint_parse_string (context_p->token.lit_location.char_p, (lit_utf8_size_t) (length - 1), options);

    JERRY_ASSERT (lit_value != ECMA_VALUE_FALSE && !ECMA_IS_VALUE_ERROR (lit_value));

    if (lit_value == ECMA_VALUE_NULL)
    {
      parser_raise_error (context_p, PARSER_ERR_OUT_OF_MEMORY);
    }

    lit_value = ecma_find_or_create_literal_bigint (lit_value);
  }
#endif /* JERRY_BUILTIN_BIGINT */

  parser_list_iterator_init (&context_p->literal_pool, &literal_iterator);

  while ((literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator)) != NULL)
  {
    if (literal_p->type == LEXER_NUMBER_LITERAL && literal_p->u.value == lit_value)
    {
      context_p->lit_object.literal_p = literal_p;
      context_p->lit_object.index = (uint16_t) literal_index;
      return false;
    }

    literal_index++;
  }

  JERRY_ASSERT (literal_index == context_p->literal_count);

  if (literal_index >= PARSER_MAXIMUM_NUMBER_OF_LITERALS)
  {
    parser_raise_error (context_p, PARSER_ERR_LITERAL_LIMIT_REACHED);
  }

  literal_p = (lexer_literal_t *) parser_list_append (context_p, &context_p->literal_pool);
  literal_p->u.value = lit_value;
  literal_p->prop.length = 0; /* Unused. */
  literal_p->type = LEXER_NUMBER_LITERAL;
  literal_p->status_flags = 0;

  context_p->lit_object.literal_p = literal_p;
  context_p->lit_object.index = (uint16_t) literal_index;

  context_p->literal_count++;
  return false;
} /* lexer_construct_number_object */