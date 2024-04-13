parser_parse_function_statement (parser_context_t *context_p) /**< context */
{
  JERRY_ASSERT (context_p->token.type == LEXER_KEYW_FUNCTION);

#if JERRY_ESNEXT
  if (JERRY_UNLIKELY (parser_statement_flags[context_p->stack_top_uint8] & PARSER_STATM_SINGLE_STATM))
  {
    if (context_p->status_flags & PARSER_IS_STRICT)
    {
      parser_raise_error (context_p, PARSER_ERR_LEXICAL_SINGLE_STATEMENT);
    }

    if (context_p->stack_top_uint8 == PARSER_STATEMENT_IF
        || context_p->stack_top_uint8 == PARSER_STATEMENT_ELSE)
    {
      /* There must be a parser error later if this check fails. */
      if (context_p->next_scanner_info_p->source_p == context_p->source_p)
      {
        parser_push_block_context (context_p, true);
      }
    }
    else if (context_p->stack_top_uint8 == PARSER_STATEMENT_LABEL)
    {
      parser_stack_iterator_t iterator;
      parser_stack_iterator_init (context_p, &iterator);
      parser_stack_iterator_skip (&iterator, sizeof (parser_label_statement_t) + 1);

      while (true)
      {
        uint8_t type = parser_stack_iterator_read_uint8 (&iterator);

        if (type == PARSER_STATEMENT_LABEL)
        {
          parser_stack_iterator_skip (&iterator, sizeof (parser_label_statement_t) + 1);
          continue;
        }

        if (parser_statement_flags[type] & PARSER_STATM_HAS_BLOCK)
        {
          break;
        }

        parser_raise_error (context_p, PARSER_ERR_LABELLED_FUNC_NOT_IN_BLOCK);
      }
    }
    else
    {
      parser_raise_error (context_p, PARSER_ERR_LEXICAL_SINGLE_STATEMENT);
    }
  }
#endif /* JERRY_ESNEXT */

#if JERRY_FUNCTION_TO_STRING
#if JERRY_ESNEXT
  if (!(context_p->next_scanner_info_p->u8_arg & SCANNER_FUNCTION_ASYNC))
  {
    context_p->function_start_p = context_p->token.lit_location.char_p;
  }
#else /* !JERRY_ESNEXT */
  context_p->function_start_p = context_p->token.lit_location.char_p;
#endif /* JERRY_ESNEXT */
#endif /* JERRY_FUNCTION_TO_STRING */

#if JERRY_DEBUGGER
  parser_line_counter_t debugger_line = context_p->token.line;
  parser_line_counter_t debugger_column = context_p->token.column;
#endif /* JERRY_DEBUGGER */

#if JERRY_ESNEXT
  bool is_generator_function = false;

  if (lexer_consume_generator (context_p))
  {
    is_generator_function = true;
  }
#endif /* JERRY_ESNEXT */

  lexer_expect_identifier (context_p, LEXER_NEW_IDENT_LITERAL);
  JERRY_ASSERT (context_p->token.type == LEXER_LITERAL
                && context_p->token.lit_location.type == LEXER_IDENT_LITERAL);

#if JERRY_ESNEXT
  if (context_p->next_scanner_info_p->source_p == context_p->source_p
      && context_p->next_scanner_info_p->type == SCANNER_TYPE_ERR_REDECLARED)
  {
    parser_raise_error (context_p, PARSER_ERR_VARIABLE_REDECLARED);
  }

  uint16_t function_name_index = context_p->lit_object.index;
#endif /* JERRY_ESNEXT */

#if JERRY_MODULE_SYSTEM
  parser_module_append_export_name (context_p);
  context_p->status_flags &= (uint32_t) ~(PARSER_MODULE_STORE_IDENT);
#endif /* JERRY_MODULE_SYSTEM */

  uint32_t status_flags = PARSER_FUNCTION_CLOSURE;

  if (context_p->token.keyword_type >= LEXER_FIRST_NON_STRICT_ARGUMENTS)
  {
    status_flags |= PARSER_HAS_NON_STRICT_ARG;
  }

#if JERRY_ESNEXT
  if (is_generator_function)
  {
    status_flags |= PARSER_IS_GENERATOR_FUNCTION | PARSER_DISALLOW_AWAIT_YIELD;
  }

  if (context_p->next_scanner_info_p->u8_arg & SCANNER_FUNCTION_ASYNC)
  {
    status_flags |= PARSER_IS_ASYNC_FUNCTION | PARSER_DISALLOW_AWAIT_YIELD;
  }
#endif /* JERRY_ESNEXT */

#if JERRY_DEBUGGER
  if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
  {
    lexer_literal_t *name_p = context_p->lit_object.literal_p;
    jerry_debugger_send_string (JERRY_DEBUGGER_FUNCTION_NAME,
                                JERRY_DEBUGGER_NO_SUBTYPE,
                                name_p->u.char_p,
                                name_p->prop.length);

    /* Reset token position for the function. */
    context_p->token.line = debugger_line;
    context_p->token.column = debugger_column;
  }
#endif /* JERRY_DEBUGGER */

  JERRY_ASSERT (context_p->scope_stack_top >= 2);
  parser_scope_stack_t *scope_stack_p = context_p->scope_stack_p + context_p->scope_stack_top - 2;

  uint16_t literal_index = context_p->lit_object.index;

  while (literal_index != scope_stack_p->map_from)
  {
    scope_stack_p--;

    JERRY_ASSERT (scope_stack_p >= context_p->scope_stack_p);
  }

  JERRY_ASSERT (scope_stack_p[1].map_from == PARSER_SCOPE_STACK_FUNC);

#if JERRY_ESNEXT
  if (!(context_p->status_flags & PARSER_IS_STRICT)
      && (scope_stack_p >= context_p->scope_stack_p + context_p->scope_stack_global_end))
  {
    bool copy_value = true;

    parser_scope_stack_t *stack_p = context_p->scope_stack_p;

    while (stack_p < scope_stack_p)
    {
      if (literal_index == stack_p->map_from
          && (stack_p->map_to & PARSER_SCOPE_STACK_NO_FUNCTION_COPY))
      {
        copy_value = false;
        break;
      }
      stack_p++;
    }

    if (copy_value)
    {
      stack_p = context_p->scope_stack_p;

      while (stack_p < scope_stack_p)
      {
        if (literal_index == stack_p->map_from)
        {
          JERRY_ASSERT (!(stack_p->map_to & PARSER_SCOPE_STACK_NO_FUNCTION_COPY));

          uint16_t map_to = scanner_decode_map_to (stack_p);
          uint16_t opcode = ((map_to >= PARSER_REGISTER_START) ? CBC_ASSIGN_LITERAL_SET_IDENT
                                                               : CBC_COPY_TO_GLOBAL);

          parser_emit_cbc_literal_value (context_p,
                                         opcode,
                                         scanner_decode_map_to (scope_stack_p),
                                         map_to);
          break;
        }
        stack_p++;
      }

      parser_flush_cbc (context_p);
    }

    if (JERRY_UNLIKELY (context_p->stack_top_uint8 == PARSER_STATEMENT_PRIVATE_SCOPE
                        || context_p->stack_top_uint8 == PARSER_STATEMENT_PRIVATE_CONTEXT))
    {
      parser_pop_block_context (context_p);
    }
  }
#endif /* JERRY_ESNEXT */

  lexer_literal_t *literal_p = PARSER_GET_LITERAL ((size_t) scope_stack_p[1].map_to);

  JERRY_ASSERT ((literal_p->type == LEXER_UNUSED_LITERAL || literal_p->type == LEXER_FUNCTION_LITERAL)
                && literal_p->status_flags == 0);

  ecma_compiled_code_t *compiled_code_p = parser_parse_function (context_p, status_flags);

  if (literal_p->type == LEXER_FUNCTION_LITERAL)
  {
    ecma_bytecode_deref (literal_p->u.bytecode_p);
  }

  literal_p->u.bytecode_p = compiled_code_p;
  literal_p->type = LEXER_FUNCTION_LITERAL;

#if JERRY_ESNEXT
  parser_compiled_code_set_function_name (context_p, compiled_code_p, function_name_index, 0);
#endif /* JERRY_ESNEXT */

  lexer_next_token (context_p);
} /* parser_parse_function_statement */