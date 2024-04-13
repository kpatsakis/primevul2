lexer_consume_generator (parser_context_t *context_p) /**< context */
{
  if (!(context_p->token.flags & LEXER_NO_SKIP_SPACES))
  {
    lexer_skip_spaces (context_p);
    context_p->token.flags = (uint8_t) (context_p->token.flags | LEXER_NO_SKIP_SPACES);
  }

  if (context_p->source_p >= context_p->source_end_p || context_p->source_p[0] != LIT_CHAR_ASTERISK
      || (context_p->source_p + 1 < context_p->source_end_p
          && (context_p->source_p[1] == LIT_CHAR_EQUALS || context_p->source_p[1] == LIT_CHAR_ASTERISK)))
  {
    return false;
  }

  lexer_consume_next_character (context_p);
  context_p->token.type = LEXER_MULTIPLY;
  return true;
} /* lexer_consume_generator */