lexer_string_is_directive (parser_context_t *context_p) /**< context */
{
  return (context_p->token.type == LEXER_SEMICOLON || context_p->token.type == LEXER_RIGHT_BRACE
          || context_p->token.type == LEXER_EOS
          || ((context_p->token.flags & LEXER_WAS_NEWLINE) && !LEXER_IS_BINARY_OP_TOKEN (context_p->token.type)
              && context_p->token.type != LEXER_LEFT_PAREN && context_p->token.type != LEXER_LEFT_SQUARE
              && context_p->token.type != LEXER_DOT));
} /* lexer_string_is_directive */