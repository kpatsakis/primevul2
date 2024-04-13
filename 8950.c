lexer_check_next_characters (parser_context_t *context_p, /**< context */
                             lit_utf8_byte_t character1, /**< first alternative character */
                             lit_utf8_byte_t character2) /**< second alternative character */
{
  if (!(context_p->token.flags & LEXER_NO_SKIP_SPACES))
  {
    lexer_skip_spaces (context_p);
    context_p->token.flags = (uint8_t) (context_p->token.flags | LEXER_NO_SKIP_SPACES);
  }

  return (context_p->source_p < context_p->source_end_p
          && (context_p->source_p[0] == (uint8_t) character1 || context_p->source_p[0] == (uint8_t) character2));
} /* lexer_check_next_characters */