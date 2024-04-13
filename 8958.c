lexer_check_arrow_param (parser_context_t *context_p) /**< context */
{
  JERRY_ASSERT (context_p->token.flags & LEXER_NO_SKIP_SPACES);

  if (context_p->source_p >= context_p->source_end_p)
  {
    return false;
  }

  if (context_p->source_p[0] == LIT_CHAR_COMMA)
  {
    return true;
  }

  if (context_p->source_p[0] != LIT_CHAR_EQUALS)
  {
    return false;
  }

  return (context_p->source_p + 1 >= context_p->source_end_p || context_p->source_p[1] != LIT_CHAR_EQUALS);
} /* lexer_check_arrow_param */