lexer_scan_private_identifier (parser_context_t *context_p) /**< context */
{
  context_p->token.keyword_type = LEXER_EOS;
  context_p->token.line = context_p->line;
  context_p->token.column = context_p->column;

  return (context_p->source_p < context_p->source_end_p && lexer_parse_identifier (context_p, LEXER_PARSE_NO_OPTS));
} /* lexer_scan_private_identifier */