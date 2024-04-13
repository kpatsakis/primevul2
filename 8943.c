lexer_check_property_modifier (parser_context_t *context_p) /**< context */
{
  JERRY_ASSERT (!(context_p->token.flags & LEXER_NO_SKIP_SPACES));
  JERRY_ASSERT (context_p->token.type == LEXER_LITERAL && context_p->token.lit_location.type == LEXER_IDENT_LITERAL);

  lexer_skip_spaces (context_p);
  context_p->token.flags = (uint8_t) (context_p->token.flags | LEXER_NO_SKIP_SPACES);

  if (context_p->source_p >= context_p->source_end_p
#if JERRY_ESNEXT
      || context_p->source_p[0] == LIT_CHAR_COMMA || context_p->source_p[0] == LIT_CHAR_RIGHT_BRACE
      || context_p->source_p[0] == LIT_CHAR_LEFT_PAREN || context_p->source_p[0] == LIT_CHAR_EQUALS
#endif /* JERRY_ESNEXT */
      || context_p->source_p[0] == LIT_CHAR_COLON)
  {
    return;
  }

  if (lexer_compare_literal_to_string (context_p, "get", 3))
  {
    context_p->token.type = LEXER_PROPERTY_GETTER;
    return;
  }

  if (lexer_compare_literal_to_string (context_p, "set", 3))
  {
    context_p->token.type = LEXER_PROPERTY_SETTER;
    return;
  }

#if JERRY_ESNEXT
  if (lexer_compare_literal_to_string (context_p, "async", 5))
  {
    context_p->token.type = LEXER_KEYW_ASYNC;
    return;
  }
#endif /* JERRY_ESNEXT */
} /* lexer_check_property_modifier */