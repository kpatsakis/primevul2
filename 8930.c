lexer_construct_class_static_block_function (parser_context_t *context_p) /**< context */
{
  ecma_compiled_code_t *compiled_code_p;
  lexer_literal_t *literal_p;
  uint16_t result_index;

  literal_p = lexer_construct_unused_literal (context_p);
  result_index = context_p->literal_count;
  context_p->literal_count++;

  parser_flush_cbc (context_p);
  compiled_code_p = parser_parse_class_static_block (context_p);

  literal_p->u.bytecode_p = compiled_code_p;
  literal_p->type = LEXER_FUNCTION_LITERAL;

  return result_index;
} /* lexer_construct_class_static_block_function */