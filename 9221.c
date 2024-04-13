parser_parse_statements (parser_context_t *context_p) /**< context */
{
  /* Statement parsing cannot be nested. */
  JERRY_ASSERT (context_p->last_statement.current_p == NULL);
  parser_stack_push_uint8 (context_p, PARSER_STATEMENT_START);
  parser_stack_iterator_init (context_p, &context_p->last_statement);

#if JERRY_DEBUGGER
  /* Set lexical enviroment for the debugger. */
  if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
  {
    context_p->status_flags |= PARSER_LEXICAL_ENV_NEEDED;
    context_p->last_breakpoint_line = 0;
  }
#endif /* JERRY_DEBUGGER */

  while (context_p->token.type == LEXER_LITERAL
         && context_p->token.lit_location.type == LEXER_STRING_LITERAL)
  {
    lexer_lit_location_t lit_location;
    parser_strict_mode_type_t strict_mode = PARSER_USE_STRICT_NOT_FOUND;

    JERRY_ASSERT (context_p->stack_depth <= 1);
#ifndef JERRY_NDEBUG
    JERRY_ASSERT (context_p->context_stack_depth == context_p->stack_depth);
#endif /* !JERRY_NDEBUG */

    if (lexer_string_is_use_strict (context_p))
    {
      strict_mode = PARSER_USE_STRICT_FOUND;

      if (!(context_p->status_flags & PARSER_IS_STRICT))
      {
        /* The next token should be parsed in strict mode. */
        context_p->status_flags |= PARSER_IS_STRICT;
        strict_mode = PARSER_USE_STRICT_SET;
      }
    }

    lit_location = context_p->token.lit_location;
    lexer_next_token (context_p);

    if (!lexer_string_is_directive (context_p))
    {
      /* The string is part of an expression statement. */
      if (strict_mode == PARSER_USE_STRICT_SET)
      {
        context_p->status_flags &= (uint32_t) ~PARSER_IS_STRICT;
      }

#if JERRY_DEBUGGER
      if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
      {
        JERRY_ASSERT (context_p->last_breakpoint_line == 0);

        parser_emit_cbc (context_p, CBC_BREAKPOINT_DISABLED);
        parser_flush_cbc (context_p);

        parser_append_breakpoint_info (context_p, JERRY_DEBUGGER_BREAKPOINT_LIST, context_p->token.line);

        context_p->last_breakpoint_line = context_p->token.line;
      }
#endif /* JERRY_DEBUGGER */
#if JERRY_LINE_INFO
      parser_line_info_append (context_p, context_p->token.line, context_p->token.column);
#endif /* JERRY_LINE_INFO */

      lexer_construct_literal_object (context_p, &lit_location, LEXER_STRING_LITERAL);
      parser_emit_cbc_literal_from_token (context_p, CBC_PUSH_LITERAL);
      /* The extra_value is used for saving the token. */
      context_p->token.extra_value = context_p->token.type;
      context_p->token.type = LEXER_EXPRESSION_START;
      break;
    }

#if JERRY_PARSER_DUMP_BYTE_CODE
    if (strict_mode == PARSER_USE_STRICT_SET && context_p->is_show_opcodes)
    {
      JERRY_DEBUG_MSG ("  Note: switch to strict mode\n\n");
    }
#endif /* JERRY_PARSER_DUMP_BYTE_CODE */

#if JERRY_ESNEXT
    if (strict_mode != PARSER_USE_STRICT_NOT_FOUND
        && (context_p->status_flags & PARSER_FUNCTION_HAS_COMPLEX_ARGUMENT))
    {
      parser_raise_error (context_p, PARSER_ERR_USE_STRICT_NOT_ALLOWED);
    }
#endif /* JERRY_ESNEXT */

    if (context_p->token.type == LEXER_SEMICOLON)
    {
      lexer_next_token (context_p);
    }

    /* The last directive prologue can be the result of the script. */
    if (!(context_p->status_flags & PARSER_IS_FUNCTION)
        && (context_p->token.type != LEXER_LITERAL
            || context_p->token.lit_location.type != LEXER_STRING_LITERAL))
    {
      lexer_construct_literal_object (context_p, &lit_location, LEXER_STRING_LITERAL);
      parser_emit_cbc_literal_from_token (context_p, CBC_PUSH_LITERAL);
      parser_emit_cbc (context_p, CBC_POP_BLOCK);
      parser_flush_cbc (context_p);
      break;
    }
  }

  if (context_p->status_flags & PARSER_IS_STRICT
      && context_p->status_flags & PARSER_HAS_NON_STRICT_ARG)
  {
    parser_raise_error (context_p, PARSER_ERR_NON_STRICT_ARG_DEFINITION);
  }

  while (context_p->token.type != LEXER_EOS
         || context_p->stack_top_uint8 != PARSER_STATEMENT_START)
  {
#ifndef JERRY_NDEBUG
    JERRY_ASSERT (context_p->stack_depth == context_p->context_stack_depth);
#endif /* !JERRY_NDEBUG */

#if JERRY_ESNEXT
    JERRY_ASSERT (context_p->stack_top_uint8 != PARSER_STATEMENT_PRIVATE_SCOPE
                  && context_p->stack_top_uint8 != PARSER_STATEMENT_PRIVATE_CONTEXT);
#endif /* JERRY_ESNEXT */

#if JERRY_DEBUGGER
    if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED
        && context_p->token.line != context_p->last_breakpoint_line
        && context_p->token.type != LEXER_SEMICOLON
        && context_p->token.type != LEXER_LEFT_BRACE
        && context_p->token.type != LEXER_RIGHT_BRACE
        && context_p->token.type != LEXER_KEYW_VAR
        && context_p->token.type != LEXER_KEYW_LET
        && context_p->token.type != LEXER_KEYW_CONST
        && context_p->token.type != LEXER_KEYW_FUNCTION
        && context_p->token.type != LEXER_KEYW_CASE
        && context_p->token.type != LEXER_KEYW_DEFAULT)
    {
      parser_emit_cbc (context_p, CBC_BREAKPOINT_DISABLED);
      parser_flush_cbc (context_p);

      parser_append_breakpoint_info (context_p, JERRY_DEBUGGER_BREAKPOINT_LIST, context_p->token.line);

      context_p->last_breakpoint_line = context_p->token.line;
    }
#endif /* JERRY_DEBUGGER */

#if JERRY_LINE_INFO
    if (context_p->token.type != LEXER_SEMICOLON
        && context_p->token.type != LEXER_LEFT_BRACE
        && context_p->token.type != LEXER_RIGHT_BRACE
        && context_p->token.type != LEXER_KEYW_VAR
        && context_p->token.type != LEXER_KEYW_LET
        && context_p->token.type != LEXER_KEYW_CONST
        && context_p->token.type != LEXER_KEYW_FUNCTION
        && context_p->token.type != LEXER_KEYW_CASE
        && context_p->token.type != LEXER_KEYW_DEFAULT)
    {
      parser_line_info_append (context_p, context_p->token.line, context_p->token.column);
    }
#endif /* JERRY_LINE_INFO */

    switch (context_p->token.type)
    {
      case LEXER_SEMICOLON:
      {
        break;
      }

      case LEXER_RIGHT_BRACE:
      {
        if (parser_statement_flags[context_p->stack_top_uint8] & PARSER_STATM_SINGLE_STATM)
        {
          parser_raise_error (context_p, PARSER_ERR_STATEMENT_EXPECTED);
        }
        break;
      }

      case LEXER_LEFT_BRACE:
      {
#if JERRY_ESNEXT
        if (context_p->next_scanner_info_p->source_p == context_p->source_p)
        {
          parser_push_block_context (context_p, false);
        }
        else
        {
          parser_stack_push_uint8 (context_p, PARSER_STATEMENT_BLOCK);
        }
#else /* !JERRY_ESNEXT */
        parser_stack_push_uint8 (context_p, PARSER_STATEMENT_BLOCK);
#endif /* JERRY_ESNEXT */

        parser_stack_iterator_init (context_p, &context_p->last_statement);
        lexer_next_token (context_p);
        continue;
      }

      case LEXER_KEYW_VAR:
#if JERRY_ESNEXT
      case LEXER_KEYW_LET:
      case LEXER_KEYW_CONST:
#endif /* JERRY_ESNEXT */
      {
        parser_parse_var_statement (context_p);
        break;
      }

#if JERRY_ESNEXT
      case LEXER_KEYW_CLASS:
      {
        parser_validate_lexical_context (context_p);
        parser_parse_class (context_p, true);
        goto consume_last_statement;
      }
#endif /* JERRY_ESNEXT */

#if JERRY_MODULE_SYSTEM
      case LEXER_KEYW_IMPORT:
      {
        parser_parse_import_statement (context_p);
        break;
      }

      case LEXER_KEYW_EXPORT:
      {
        if (parser_parse_export_statement (context_p))
        {
          goto consume_last_statement;
        }
        break;
      }
#endif /* JERRY_MODULE_SYSTEM */

      case LEXER_KEYW_FUNCTION:
      {
        parser_parse_function_statement (context_p);
        goto consume_last_statement;
      }

      case LEXER_KEYW_IF:
      {
        parser_parse_if_statement_start (context_p);
        continue;
      }

      case LEXER_KEYW_SWITCH:
      {
        parser_parse_switch_statement_start (context_p);
        continue;
      }

      case LEXER_KEYW_DO:
      {
        parser_do_while_statement_t do_while_statement;
        parser_loop_statement_t loop;

        JERRY_ASSERT (context_p->last_cbc_opcode == PARSER_CBC_UNAVAILABLE);

        do_while_statement.start_offset = context_p->byte_code_size;
        loop.branch_list_p = NULL;

        parser_stack_push (context_p, &do_while_statement, sizeof (parser_do_while_statement_t));
        parser_stack_push (context_p, &loop, sizeof (parser_loop_statement_t));
        parser_stack_push_uint8 (context_p, PARSER_STATEMENT_DO_WHILE);
        parser_stack_iterator_init (context_p, &context_p->last_statement);
        lexer_next_token (context_p);
        continue;
      }

      case LEXER_KEYW_WHILE:
      {
        parser_parse_while_statement_start (context_p);
        continue;
      }

      case LEXER_KEYW_FOR:
      {
        parser_parse_for_statement_start (context_p);
        continue;
      }

      case LEXER_KEYW_WITH:
      {
        parser_parse_with_statement_start (context_p);
        continue;
      }

      case LEXER_KEYW_TRY:
      {
        parser_try_statement_t try_statement;

        lexer_next_token (context_p);

        if (context_p->token.type != LEXER_LEFT_BRACE)
        {
          parser_raise_error (context_p, PARSER_ERR_LEFT_BRACE_EXPECTED);
        }

#ifndef JERRY_NDEBUG
        PARSER_PLUS_EQUAL_U16 (context_p->context_stack_depth, PARSER_TRY_CONTEXT_STACK_ALLOCATION);
#endif /* !JERRY_NDEBUG */

        try_statement.type = parser_try_block;
        parser_emit_cbc_ext_forward_branch (context_p,
                                            CBC_EXT_TRY_CREATE_CONTEXT,
                                            &try_statement.branch);

#if JERRY_ESNEXT
        try_statement.scope_stack_top = context_p->scope_stack_top;
        try_statement.scope_stack_reg_top = context_p->scope_stack_reg_top;

        if (context_p->next_scanner_info_p->source_p == context_p->source_p)
        {
          JERRY_ASSERT (context_p->next_scanner_info_p->type == SCANNER_TYPE_BLOCK);

          if (scanner_is_context_needed (context_p, PARSER_CHECK_BLOCK_CONTEXT))
          {
            parser_emit_cbc_ext (context_p, CBC_EXT_TRY_CREATE_ENV);
          }

          scanner_create_variables (context_p, SCANNER_CREATE_VARS_NO_OPTS);
        }
#endif /* JERRY_ESNEXT */

        parser_stack_push (context_p, &try_statement, sizeof (parser_try_statement_t));
        parser_stack_push_uint8 (context_p, PARSER_STATEMENT_TRY);
        parser_stack_iterator_init (context_p, &context_p->last_statement);
        lexer_next_token (context_p);
        continue;
      }

      case LEXER_KEYW_DEFAULT:
      {
        parser_parse_default_statement (context_p);
        continue;
      }

      case LEXER_KEYW_CASE:
      {
        parser_parse_case_statement (context_p);
        continue;
      }

      case LEXER_KEYW_BREAK:
      {
        parser_parse_break_statement (context_p);
        break;
      }

      case LEXER_KEYW_CONTINUE:
      {
        parser_parse_continue_statement (context_p);
        break;
      }

      case LEXER_KEYW_THROW:
      {
        lexer_next_token (context_p);
        if (context_p->token.flags & LEXER_WAS_NEWLINE)
        {
          parser_raise_error (context_p, PARSER_ERR_EXPRESSION_EXPECTED);
        }
        parser_parse_expression (context_p, PARSE_EXPR);
        parser_emit_cbc (context_p, CBC_THROW);
        break;
      }

      case LEXER_KEYW_RETURN:
      {
        if (!(context_p->status_flags & PARSER_IS_FUNCTION))
        {
          parser_raise_error (context_p, PARSER_ERR_INVALID_RETURN);
        }

        lexer_next_token (context_p);

        if ((context_p->token.flags & LEXER_WAS_NEWLINE)
            || context_p->token.type == LEXER_SEMICOLON
            || context_p->token.type == LEXER_EOS
            || context_p->token.type == LEXER_RIGHT_BRACE)
        {
#if JERRY_ESNEXT
          if (context_p->status_flags & PARSER_IS_ASYNC_FUNCTION)
          {
            parser_emit_cbc_ext (context_p, CBC_EXT_RETURN_UNDEFINED);
            break;
          }
#endif /* JERRY_ESNEXT */

          parser_emit_cbc (context_p, CBC_RETURN_FUNCTION_END);
          break;
        }

        parser_parse_expression (context_p, PARSE_EXPR);

        if (context_p->last_cbc_opcode == CBC_PUSH_LITERAL)
        {
          context_p->last_cbc_opcode = CBC_RETURN_WITH_LITERAL;
          break;
        }

        parser_emit_cbc (context_p, CBC_RETURN);
        break;
      }

      case LEXER_KEYW_DEBUGGER:
      {
#if JERRY_DEBUGGER
        /* This breakpoint location is not reported to the
         * debugger, so it is impossible to disable it. */
        if (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_CONNECTED)
        {
          parser_emit_cbc (context_p, CBC_BREAKPOINT_ENABLED);
        }
#endif /* JERRY_DEBUGGER */
        lexer_next_token (context_p);
        break;
      }

      case LEXER_LITERAL:
      {
        if (context_p->token.lit_location.type == LEXER_IDENT_LITERAL)
        {
          if (JERRY_UNLIKELY (lexer_check_next_character (context_p, LIT_CHAR_COLON)))
          {
            parser_parse_label (context_p);
            lexer_consume_next_character (context_p);
            lexer_next_token (context_p);
            continue;
          }
#if JERRY_ESNEXT
          if (JERRY_UNLIKELY (lexer_token_is_let (context_p)))
          {
            if (context_p->next_scanner_info_p->source_p == context_p->source_p)
            {
              if (context_p->next_scanner_info_p->type == SCANNER_TYPE_LET_EXPRESSION)
              {
                scanner_release_next (context_p, sizeof (scanner_info_t));
              }

              if (context_p->status_flags & PARSER_IS_FUNCTION)
              {
                parser_parse_expression_statement (context_p, PARSE_EXPR);
                break;
              }

              parser_parse_block_expression (context_p, PARSE_EXPR);
              break;
            }

            context_p->token.type = LEXER_KEYW_LET;
            parser_parse_var_statement (context_p);
            break;
          }

          if (JERRY_UNLIKELY (lexer_token_is_async (context_p))
              && context_p->next_scanner_info_p->source_p == context_p->source_p)
          {
            bool is_statement = true;

            if (context_p->next_scanner_info_p->type == SCANNER_TYPE_FUNCTION)
            {
              is_statement = (context_p->next_scanner_info_p->u8_arg & SCANNER_FUNCTION_STATEMENT) != 0;

              JERRY_ASSERT (!is_statement || (context_p->next_scanner_info_p->u8_arg & SCANNER_FUNCTION_ASYNC));
            }
            else
            {
              JERRY_ASSERT (context_p->next_scanner_info_p->type == SCANNER_TYPE_ERR_ASYNC_FUNCTION);

              scanner_release_next (context_p, sizeof (scanner_info_t));
            }

            if (is_statement)
            {
              if (parser_statement_flags[context_p->stack_top_uint8] & PARSER_STATM_SINGLE_STATM)
              {
                parser_raise_error (context_p, PARSER_ERR_LEXICAL_SINGLE_STATEMENT);
              }

#if JERRY_FUNCTION_TO_STRING
              context_p->function_start_p = context_p->token.lit_location.char_p;
#endif /* JERRY_FUNCTION_TO_STRING */
              lexer_next_token (context_p);
              JERRY_ASSERT (context_p->token.type == LEXER_KEYW_FUNCTION);
              continue;
            }
          }
#endif /* JERRY_ESNEXT */
        }
        /* FALLTHRU */
      }

      default:
      {
        int options = PARSE_EXPR;

        if (context_p->token.type == LEXER_EXPRESSION_START)
        {
          /* Restore the token type form the extra_value. */
          context_p->token.type = context_p->token.extra_value;
          options |= PARSE_EXPR_HAS_LITERAL;
        }

        if (context_p->status_flags & PARSER_IS_FUNCTION)
        {
          parser_parse_expression_statement (context_p, options);
        }
        else
        {
          parser_parse_block_expression (context_p, options);
        }

        break;
      }
    }

    parser_flush_cbc (context_p);

    if (context_p->token.type == LEXER_RIGHT_BRACE)
    {
      if (context_p->stack_top_uint8 == PARSER_STATEMENT_BLOCK)
      {
        parser_stack_pop_uint8 (context_p);
        parser_stack_iterator_init (context_p, &context_p->last_statement);
        lexer_next_token (context_p);
      }
#if JERRY_ESNEXT
      else if (context_p->stack_top_uint8 == PARSER_STATEMENT_BLOCK_SCOPE
               || context_p->stack_top_uint8 == PARSER_STATEMENT_BLOCK_CONTEXT)
      {
        parser_pop_block_context (context_p);
        lexer_next_token (context_p);
      }
#endif /* JERRY_ESNEXT */
      else if (context_p->stack_top_uint8 == PARSER_STATEMENT_SWITCH
               || context_p->stack_top_uint8 == PARSER_STATEMENT_SWITCH_NO_DEFAULT)
      {
        int has_default = (context_p->stack_top_uint8 == PARSER_STATEMENT_SWITCH);
        parser_loop_statement_t loop;
        parser_switch_statement_t switch_statement;

        parser_stack_pop_uint8 (context_p);
        parser_stack_pop (context_p, &loop, sizeof (parser_loop_statement_t));
        parser_stack_pop (context_p, &switch_statement, sizeof (parser_switch_statement_t));
        parser_stack_iterator_init (context_p, &context_p->last_statement);

        JERRY_ASSERT (switch_statement.branch_list_p == NULL);

        if (!has_default)
        {
          parser_set_branch_to_current_position (context_p, &switch_statement.default_branch);
        }

        parser_set_breaks_to_current_position (context_p, loop.branch_list_p);
        lexer_next_token (context_p);

#if JERRY_ESNEXT
        if (context_p->stack_top_uint8 == PARSER_STATEMENT_PRIVATE_SCOPE
            || context_p->stack_top_uint8 == PARSER_STATEMENT_PRIVATE_CONTEXT)
        {
          parser_pop_block_context (context_p);
        }
#endif /* JERRY_ESNEXT */
      }
      else if (context_p->stack_top_uint8 == PARSER_STATEMENT_TRY)
      {
        parser_parse_try_statement_end (context_p);
      }
      else if (context_p->stack_top_uint8 == PARSER_STATEMENT_START)
      {
        if (context_p->status_flags & PARSER_IS_CLOSURE)
        {
#if JERRY_LINE_INFO
          if (context_p->line_info_p == NULL)
          {
            parser_line_info_append (context_p, context_p->token.line, context_p->token.column);
          }
#endif /* JERRY_LINE_INFO */

#if JERRY_FUNCTION_TO_STRING
          context_p->function_end_p = context_p->source_p;
#endif /* JERRY_FUNCTION_TO_STRING */

          parser_stack_pop_uint8 (context_p);
          context_p->last_statement.current_p = NULL;
          /* There is no lexer_next_token here, since the
           * next token belongs to the parent context. */
          return;
        }
        parser_raise_error (context_p, PARSER_ERR_INVALID_RIGHT_SQUARE);
      }
    }
    else if (context_p->token.type == LEXER_SEMICOLON)
    {
      lexer_next_token (context_p);
    }
    else if (context_p->token.type != LEXER_EOS
             && !(context_p->token.flags & LEXER_WAS_NEWLINE))
    {
      parser_raise_error (context_p, PARSER_ERR_SEMICOLON_EXPECTED);
    }

consume_last_statement:
    while (true)
    {
      switch (context_p->stack_top_uint8)
      {
        case PARSER_STATEMENT_LABEL:
        {
          parser_label_statement_t label;

          parser_stack_pop_uint8 (context_p);
          parser_stack_pop (context_p, &label, sizeof (parser_label_statement_t));
          parser_stack_iterator_init (context_p, &context_p->last_statement);

          parser_set_breaks_to_current_position (context_p, label.break_list_p);
          continue;
        }

        case PARSER_STATEMENT_IF:
        {
          if (parser_parse_if_statement_end (context_p))
          {
            break;
          }
          continue;
        }

        case PARSER_STATEMENT_ELSE:
        {
          parser_if_else_statement_t else_statement;

          parser_stack_pop_uint8 (context_p);
          parser_stack_pop (context_p, &else_statement, sizeof (parser_if_else_statement_t));
          parser_stack_iterator_init (context_p, &context_p->last_statement);

          parser_set_branch_to_current_position (context_p, &else_statement.branch);
          continue;
        }

        case PARSER_STATEMENT_DO_WHILE:
        {
          parser_parse_do_while_statement_end (context_p);
          if (context_p->token.type == LEXER_SEMICOLON)
          {
            lexer_next_token (context_p);
          }
          continue;
        }

        case PARSER_STATEMENT_WHILE:
        {
          parser_parse_while_statement_end (context_p);
          continue;
        }

        case PARSER_STATEMENT_FOR:
        {
          parser_parse_for_statement_end (context_p);
          continue;
        }

        case PARSER_STATEMENT_FOR_IN:
#if JERRY_ESNEXT
        case PARSER_STATEMENT_FOR_OF:
        case PARSER_STATEMENT_FOR_AWAIT_OF:
#endif /* JERRY_ESNEXT */
        {
          parser_for_in_of_statement_t for_in_of_statement;
          parser_loop_statement_t loop;

#if JERRY_ESNEXT
          uint8_t for_type = context_p->stack_top_uint8;
#endif /* JERRY_ESNEXT */

          parser_stack_pop_uint8 (context_p);
          parser_stack_pop (context_p, &loop, sizeof (parser_loop_statement_t));
          parser_stack_pop (context_p, &for_in_of_statement, sizeof (parser_for_in_of_statement_t));
          parser_stack_iterator_init (context_p, &context_p->last_statement);

          parser_set_continues_to_current_position (context_p, loop.branch_list_p);

          parser_flush_cbc (context_p);

          uint16_t stack_allocation = PARSER_FOR_IN_CONTEXT_STACK_ALLOCATION;
#if JERRY_ESNEXT
          if (for_type != PARSER_STATEMENT_FOR_IN)
          {
            stack_allocation = (for_type == PARSER_STATEMENT_FOR_OF ? PARSER_FOR_OF_CONTEXT_STACK_ALLOCATION
                                                                    : PARSER_FOR_AWAIT_OF_CONTEXT_STACK_ALLOCATION);
          }
#endif /* JERRY_ESNEXT */

          PARSER_MINUS_EQUAL_U16 (context_p->stack_depth, stack_allocation);
#ifndef JERRY_NDEBUG
          PARSER_MINUS_EQUAL_U16 (context_p->context_stack_depth, stack_allocation);
#endif /* !JERRY_NDEBUG */

          cbc_ext_opcode_t opcode = CBC_EXT_BRANCH_IF_FOR_IN_HAS_NEXT;

#if JERRY_ESNEXT
          if (for_type != PARSER_STATEMENT_FOR_IN)
          {
            opcode = (for_type == PARSER_STATEMENT_FOR_OF ? CBC_EXT_BRANCH_IF_FOR_OF_HAS_NEXT
                                                          : CBC_EXT_BRANCH_IF_FOR_AWAIT_OF_HAS_NEXT);
          }
#endif /* JERRY_ESNEXT */

          parser_emit_cbc_ext_backward_branch (context_p, opcode, for_in_of_statement.start_offset);

          parser_set_breaks_to_current_position (context_p, loop.branch_list_p);
          parser_set_branch_to_current_position (context_p, &for_in_of_statement.branch);

#if JERRY_ESNEXT
          if (context_p->stack_top_uint8 == PARSER_STATEMENT_PRIVATE_SCOPE
              || context_p->stack_top_uint8 == PARSER_STATEMENT_PRIVATE_CONTEXT)
          {
            parser_pop_block_context (context_p);
          }
#endif /* JERRY_ESNEXT */
          continue;
        }

        case PARSER_STATEMENT_WITH:
        {
          parser_parse_with_statement_end (context_p);
          continue;
        }

        default:
        {
          break;
        }
      }
      break;
    }
  }

  parser_stack_pop_uint8 (context_p);
  context_p->last_statement.current_p = NULL;

  if (context_p->status_flags & PARSER_IS_CLOSURE)
  {
    parser_raise_error (context_p, PARSER_ERR_STATEMENT_EXPECTED);
  }

#if JERRY_LINE_INFO
  if (context_p->line_info_p == NULL)
  {
    parser_line_info_append (context_p, context_p->token.line, context_p->token.column);
  }
#endif /* JERRY_LINE_INFO */
} /* parser_parse_statements */