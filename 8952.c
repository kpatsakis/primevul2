lexer_skip_empty_statements (parser_context_t *context_p) /**< context */
{
  lexer_skip_spaces (context_p);

  while (context_p->source_p < context_p->source_end_p && *context_p->source_p == LIT_CHAR_SEMICOLON)
  {
    lexer_consume_next_character (context_p);
    lexer_skip_spaces (context_p);
  }

  context_p->token.flags = (uint8_t) (context_p->token.flags | LEXER_NO_SKIP_SPACES);
} /* lexer_skip_empty_statements */