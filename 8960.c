lexer_check_arrow (parser_context_t *context_p) /**< context */
{
  if (!(context_p->token.flags & LEXER_NO_SKIP_SPACES))
  {
    lexer_skip_spaces (context_p);
    context_p->token.flags = (uint8_t) (context_p->token.flags | LEXER_NO_SKIP_SPACES);
  }

  return (!(context_p->token.flags & LEXER_WAS_NEWLINE) && context_p->source_p + 2 <= context_p->source_end_p
          && context_p->source_p[0] == (uint8_t) LIT_CHAR_EQUALS
          && context_p->source_p[1] == (uint8_t) LIT_CHAR_GREATER_THAN);
} /* lexer_check_arrow */