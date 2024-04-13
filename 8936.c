lexer_scan_identifier (parser_context_t *context_p, /**< context */
                       lexer_parse_options_t opts) /**< identifier parse options */
{
  lexer_skip_spaces (context_p);
  context_p->token.keyword_type = LEXER_EOS;
  context_p->token.line = context_p->line;
  context_p->token.column = context_p->column;

  if (context_p->source_p < context_p->source_end_p && lexer_parse_identifier (context_p, opts))
  {
    return true;
  }

  context_p->token.flags |= LEXER_NO_SKIP_SPACES;
  lexer_next_token (context_p);
  return false;
} /* lexer_scan_identifier */