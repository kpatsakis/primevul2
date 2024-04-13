lexer_convert_push_number_to_push_literal (parser_context_t *context_p) /**< context */
{
  ecma_integer_value_t value;
  bool two_literals = context_p->last_cbc_opcode >= CBC_PUSH_LITERAL_PUSH_NUMBER_0;

  if (context_p->last_cbc_opcode == CBC_PUSH_NUMBER_0 || context_p->last_cbc_opcode == CBC_PUSH_LITERAL_PUSH_NUMBER_0)
  {
    value = 0;
  }
  else if (context_p->last_cbc_opcode == CBC_PUSH_NUMBER_POS_BYTE
           || context_p->last_cbc_opcode == CBC_PUSH_LITERAL_PUSH_NUMBER_POS_BYTE)
  {
    value = ((ecma_integer_value_t) context_p->last_cbc.value) + 1;
  }
  else
  {
    JERRY_ASSERT (context_p->last_cbc_opcode == CBC_PUSH_NUMBER_NEG_BYTE
                  || context_p->last_cbc_opcode == CBC_PUSH_LITERAL_PUSH_NUMBER_NEG_BYTE);
    value = -((ecma_integer_value_t) context_p->last_cbc.value) - 1;
  }

  ecma_value_t lit_value = ecma_make_integer_value (value);

  parser_list_iterator_t literal_iterator;
  parser_list_iterator_init (&context_p->literal_pool, &literal_iterator);

  context_p->last_cbc_opcode = two_literals ? CBC_PUSH_TWO_LITERALS : CBC_PUSH_LITERAL;

  uint32_t literal_index = 0;
  lexer_literal_t *literal_p;

  while ((literal_p = (lexer_literal_t *) parser_list_iterator_next (&literal_iterator)) != NULL)
  {
    if (literal_p->type == LEXER_NUMBER_LITERAL && literal_p->u.value == lit_value)
    {
      if (two_literals)
      {
        context_p->last_cbc.value = (uint16_t) literal_index;
      }
      else
      {
        context_p->last_cbc.literal_index = (uint16_t) literal_index;
      }
      return;
    }

    literal_index++;
  }

  JERRY_ASSERT (literal_index == context_p->literal_count);

  if (literal_index >= PARSER_MAXIMUM_NUMBER_OF_LITERALS)
  {
    parser_raise_error (context_p, PARSER_ERR_LITERAL_LIMIT_REACHED);
  }

  literal_p = (lexer_literal_t *) parser_list_append (context_p, &context_p->literal_pool);
  literal_p->u.value = lit_value;
  literal_p->prop.length = 0; /* Unused. */
  literal_p->type = LEXER_NUMBER_LITERAL;
  literal_p->status_flags = 0;

  context_p->literal_count++;

  if (two_literals)
  {
    context_p->last_cbc.value = (uint16_t) literal_index;
  }
  else
  {
    context_p->last_cbc.literal_index = (uint16_t) literal_index;
  }
} /* lexer_convert_push_number_to_push_literal */