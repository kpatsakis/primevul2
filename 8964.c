lexer_check_post_primary_exp (parser_context_t *context_p) /**< context */
{
  if (!(context_p->token.flags & LEXER_NO_SKIP_SPACES))
  {
    lexer_skip_spaces (context_p);
    context_p->token.flags = (uint8_t) (context_p->token.flags | LEXER_NO_SKIP_SPACES);
  }

  if (context_p->source_p >= context_p->source_end_p)
  {
    return false;
  }

  switch (context_p->source_p[0])
  {
    case LIT_CHAR_DOT:
    case LIT_CHAR_LEFT_PAREN:
    case LIT_CHAR_LEFT_SQUARE:
#if JERRY_ESNEXT
    case LIT_CHAR_GRAVE_ACCENT:
#endif /* JERRY_ESNEXT */
    {
      return true;
    }
    case LIT_CHAR_PLUS:
    case LIT_CHAR_MINUS:
    {
      return (!(context_p->token.flags & LEXER_WAS_NEWLINE) && context_p->source_p + 1 < context_p->source_end_p
              && context_p->source_p[1] == context_p->source_p[0]);
    }
#if JERRY_ESNEXT
    case LIT_CHAR_ASTERISK:
    {
      return (context_p->source_p + 1 < context_p->source_end_p
              && context_p->source_p[1] == (uint8_t) LIT_CHAR_ASTERISK);
    }
#endif /* JERRY_ESNEXT */
  }

  return false;
} /* lexer_check_post_primary_exp */