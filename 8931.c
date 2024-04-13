lexer_expect_identifier (parser_context_t *context_p, /**< context */
                         uint8_t literal_type) /**< literal type */
{
  JERRY_ASSERT (literal_type == LEXER_STRING_LITERAL || literal_type == LEXER_IDENT_LITERAL
                || literal_type == LEXER_NEW_IDENT_LITERAL);

  lexer_skip_spaces (context_p);
  context_p->token.keyword_type = LEXER_EOS;
  context_p->token.line = context_p->line;
  context_p->token.column = context_p->column;

  if (context_p->source_p < context_p->source_end_p
      && lexer_parse_identifier (
        context_p,
        (literal_type != LEXER_STRING_LITERAL ? LEXER_PARSE_CHECK_KEYWORDS : LEXER_PARSE_NO_OPTS)))
  {
    if (context_p->token.type == LEXER_LITERAL)
    {
      JERRY_ASSERT (context_p->token.lit_location.type == LEXER_IDENT_LITERAL);

      lexer_construct_literal_object (context_p, &context_p->token.lit_location, literal_type);

      if (literal_type != LEXER_STRING_LITERAL && (context_p->status_flags & PARSER_IS_STRICT))
      {
        if (context_p->token.keyword_type == LEXER_KEYW_EVAL)
        {
          parser_raise_error (context_p, PARSER_ERR_EVAL_NOT_ALLOWED);
        }
        else if (context_p->token.keyword_type == LEXER_KEYW_ARGUMENTS)
        {
          parser_raise_error (context_p, PARSER_ERR_ARGUMENTS_NOT_ALLOWED);
        }
      }
      return;
    }
  }
#if JERRY_MODULE_SYSTEM
  else if (context_p->status_flags & PARSER_MODULE_DEFAULT_CLASS_OR_FUNC)
  {
    /* When parsing default exports for modules, it is not required by functions or classes to have identifiers.
     * In this case we use a synthetic name for them. */
    context_p->token.type = LEXER_LITERAL;
    context_p->token.lit_location = lexer_default_literal;
    lexer_construct_literal_object (context_p, &context_p->token.lit_location, literal_type);
    context_p->status_flags &= (uint32_t) ~(PARSER_MODULE_DEFAULT_CLASS_OR_FUNC);
    return;
  }
#endif /* JERRY_MODULE_SYSTEM */

#if JERRY_ESNEXT
  if (context_p->token.type == LEXER_KEYW_YIELD)
  {
    parser_raise_error (context_p, PARSER_ERR_YIELD_NOT_ALLOWED);
  }
  if (context_p->token.type == LEXER_KEYW_AWAIT)
  {
    parser_raise_error (context_p, PARSER_ERR_AWAIT_NOT_ALLOWED);
  }
#endif /* JERRY_ESNEXT */
  parser_raise_error (context_p, PARSER_ERR_IDENTIFIER_EXPECTED);
} /* lexer_expect_identifier */