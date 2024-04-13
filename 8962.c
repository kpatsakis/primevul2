lexer_construct_regexp_object (parser_context_t *context_p, /**< context */
                               bool parse_only) /**< parse only */
{
#if JERRY_BUILTIN_REGEXP
  const uint8_t *source_p = context_p->source_p;
  const uint8_t *regex_start_p = context_p->source_p;
  const uint8_t *regex_end_p = regex_start_p;
  const uint8_t *source_end_p = context_p->source_end_p;
  parser_line_counter_t column = context_p->column;
  bool in_class = false;
  uint16_t current_flags;
  lit_utf8_size_t length;

  JERRY_ASSERT (context_p->token.type == LEXER_DIVIDE || context_p->token.type == LEXER_ASSIGN_DIVIDE);

  if (context_p->token.type == LEXER_ASSIGN_DIVIDE)
  {
    regex_start_p--;
  }

  while (true)
  {
    if (source_p >= source_end_p)
    {
      parser_raise_error (context_p, PARSER_ERR_UNTERMINATED_REGEXP);
    }

    if (!in_class && source_p[0] == LIT_CHAR_SLASH)
    {
      regex_end_p = source_p;
      source_p++;
      column++;
      break;
    }

    switch (source_p[0])
    {
      case LIT_CHAR_CR:
      case LIT_CHAR_LF:
      case LEXER_NEWLINE_LS_PS_BYTE_1:
      {
        if (source_p[0] != LEXER_NEWLINE_LS_PS_BYTE_1 || LEXER_NEWLINE_LS_PS_BYTE_23 (source_p))
        {
          parser_raise_error (context_p, PARSER_ERR_NEWLINE_NOT_ALLOWED);
        }
        break;
      }
      case LIT_CHAR_TAB:
      {
        column = align_column_to_tab (column);
        /* Subtract -1 because column is increased below. */
        column--;
        break;
      }
      case LIT_CHAR_LEFT_SQUARE:
      {
        in_class = true;
        break;
      }
      case LIT_CHAR_RIGHT_SQUARE:
      {
        in_class = false;
        break;
      }
      case LIT_CHAR_BACKSLASH:
      {
        if (source_p + 1 >= source_end_p)
        {
          parser_raise_error (context_p, PARSER_ERR_UNTERMINATED_REGEXP);
        }

        if (source_p[1] >= 0x20 && source_p[1] <= LIT_UTF8_1_BYTE_CODE_POINT_MAX)
        {
          source_p++;
          column++;
        }
      }
    }

    source_p++;
    column++;

    while (source_p < source_end_p && IS_UTF8_INTERMEDIATE_OCTET (source_p[0]))
    {
      source_p++;
    }
  }

  current_flags = 0;
  while (source_p < source_end_p)
  {
    uint32_t flag = 0;

    if (source_p[0] == LIT_CHAR_LOWERCASE_G)
    {
      flag = RE_FLAG_GLOBAL;
    }
    else if (source_p[0] == LIT_CHAR_LOWERCASE_I)
    {
      flag = RE_FLAG_IGNORE_CASE;
    }
    else if (source_p[0] == LIT_CHAR_LOWERCASE_M)
    {
      flag = RE_FLAG_MULTILINE;
    }
    else if (source_p[0] == LIT_CHAR_LOWERCASE_U)
    {
      flag = RE_FLAG_UNICODE;
    }
    else if (source_p[0] == LIT_CHAR_LOWERCASE_Y)
    {
      flag = RE_FLAG_STICKY;
    }
#if JERRY_ESNEXT
    else if (source_p[0] == LIT_CHAR_LOWERCASE_S)
    {
      flag = RE_FLAG_DOTALL;
    }
#endif /* JERRY_ESNEXT */

    if (flag == 0)
    {
      break;
    }

    if (current_flags & flag)
    {
      parser_raise_error (context_p, PARSER_ERR_DUPLICATED_REGEXP_FLAG);
    }

    current_flags = (uint16_t) (current_flags | flag);
    source_p++;
    column++;
  }

  context_p->source_p = source_p;
  context_p->column = column;

  if (source_p < source_end_p && lexer_parse_identifier (context_p, LEXER_PARSE_CHECK_PART_AND_RETURN))
  {
    parser_raise_error (context_p, PARSER_ERR_UNKNOWN_REGEXP_FLAG);
  }

  length = (lit_utf8_size_t) (regex_end_p - regex_start_p);
  if (length > PARSER_MAXIMUM_STRING_LENGTH)
  {
    parser_raise_error (context_p, PARSER_ERR_REGEXP_TOO_LONG);
  }

  context_p->column = column;
  context_p->source_p = source_p;

  if (parse_only)
  {
    return;
  }

  if (context_p->literal_count >= PARSER_MAXIMUM_NUMBER_OF_LITERALS)
  {
    parser_raise_error (context_p, PARSER_ERR_LITERAL_LIMIT_REACHED);
  }

  /* Compile the RegExp literal and store the RegExp bytecode pointer */
  ecma_string_t *pattern_str_p = NULL;

  if (lit_is_valid_cesu8_string (regex_start_p, length))
  {
    pattern_str_p = ecma_new_ecma_string_from_utf8 (regex_start_p, length);
  }
  else
  {
    JERRY_ASSERT (lit_is_valid_utf8_string (regex_start_p, length, false));
    pattern_str_p = ecma_new_ecma_string_from_utf8_converted_to_cesu8 (regex_start_p, length);
  }

  re_compiled_code_t *re_bytecode_p = re_compile_bytecode (pattern_str_p, current_flags);
  ecma_deref_ecma_string (pattern_str_p);

  if (JERRY_UNLIKELY (re_bytecode_p == NULL))
  {
    parser_raise_error (context_p, PARSER_ERR_INVALID_REGEXP);
  }

  lexer_literal_t *literal_p = (lexer_literal_t *) parser_list_append (context_p, &context_p->literal_pool);
  literal_p->u.bytecode_p = (ecma_compiled_code_t *) re_bytecode_p;
  literal_p->type = LEXER_REGEXP_LITERAL;
  literal_p->prop.length = (prop_length_t) length;
  literal_p->status_flags = 0;

  context_p->token.type = LEXER_LITERAL;
  context_p->token.lit_location.type = LEXER_REGEXP_LITERAL;

  context_p->lit_object.literal_p = literal_p;
  context_p->lit_object.index = context_p->literal_count++;
#else /* !JERRY_BUILTIN_REGEXP */
  JERRY_UNUSED (parse_only);
  parser_raise_error (context_p, PARSER_ERR_UNSUPPORTED_REGEXP);
#endif /* JERRY_BUILTIN_REGEXP */
} /* lexer_construct_regexp_object */