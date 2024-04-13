lexer_update_await_yield (parser_context_t *context_p, /**< context */
                          uint32_t status_flags) /**< parser status flags after restore */
{
  if (!(status_flags & PARSER_IS_STRICT))
  {
    if (status_flags & PARSER_IS_GENERATOR_FUNCTION)
    {
      if (context_p->token.type == LEXER_LITERAL && context_p->token.keyword_type == LEXER_KEYW_YIELD)
      {
        context_p->token.type = LEXER_KEYW_YIELD;
      }
    }
    else
    {
      if (context_p->token.type == LEXER_KEYW_YIELD)
      {
        JERRY_ASSERT (context_p->token.keyword_type == LEXER_KEYW_YIELD);
        context_p->token.type = LEXER_LITERAL;
      }
    }
  }

  if (!(context_p->global_status_flags & ECMA_PARSE_MODULE))
  {
    if (status_flags & PARSER_IS_ASYNC_FUNCTION)
    {
      if (context_p->token.type == LEXER_LITERAL && context_p->token.keyword_type == LEXER_KEYW_AWAIT)
      {
        context_p->token.type = LEXER_KEYW_AWAIT;
      }
    }
    else
    {
      if (context_p->token.type == LEXER_KEYW_AWAIT)
      {
        JERRY_ASSERT (context_p->token.keyword_type == LEXER_KEYW_AWAIT);
        context_p->token.type = LEXER_LITERAL;
      }
    }
  }
} /* lexer_update_await_yield */