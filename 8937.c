lexer_init_line_info (parser_context_t *context_p) /**< context */
{
  context_p->line = 1;
  context_p->column = 1;

  const jerry_parse_options_t *options_p = context_p->options_p;

  if (options_p != NULL && (options_p->options & JERRY_PARSE_HAS_START))
  {
    if (options_p->start_line > 0)
    {
      context_p->line = options_p->start_line;
    }

    if (options_p->start_column > 0)
    {
      context_p->column = options_p->start_column;
    }
  }
} /* lexer_init_line_info */