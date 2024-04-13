lexer_string_is_use_strict (parser_context_t *context_p) /**< context */
{
  JERRY_ASSERT (context_p->token.type == LEXER_LITERAL && context_p->token.lit_location.type == LEXER_STRING_LITERAL);

  return (context_p->token.lit_location.length == 10
          && !(context_p->token.lit_location.status_flags & LEXER_LIT_LOCATION_HAS_ESCAPE)
          && memcmp (context_p->token.lit_location.char_p, "use strict", 10) == 0);
} /* lexer_string_is_use_strict */