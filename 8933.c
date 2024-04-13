lexer_token_is_identifier (parser_context_t *context_p, /**< context */
                           const char *identifier_p, /**< identifier */
                           size_t identifier_length) /**< identifier length */
{
  /* Checking has_escape is unnecessary because memcmp will fail if escape sequences are present. */
  return (context_p->token.type == LEXER_LITERAL && context_p->token.lit_location.type == LEXER_IDENT_LITERAL
          && context_p->token.lit_location.length == identifier_length
          && memcmp (context_p->token.lit_location.char_p, identifier_p, identifier_length) == 0);
} /* lexer_token_is_identifier */