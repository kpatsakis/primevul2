opfunc_construct (vm_frame_ctx_t *frame_ctx_p) /**< frame context */
{
  const uint8_t *byte_code_p = frame_ctx_p->byte_code_p + 1;
  uint8_t opcode = byte_code_p[-1];
  unsigned int arguments_list_len;

  if (opcode >= CBC_NEW0)
  {
    arguments_list_len = (unsigned int) (opcode - CBC_NEW0);
  }
  else
  {
    arguments_list_len = *byte_code_p++;
  }

  ecma_value_t *stack_top_p = frame_ctx_p->stack_top_p - arguments_list_len;
  ecma_value_t constructor_value = stack_top_p[-1];
  ecma_value_t completion_value;

  const char *constructor_message_p = ecma_check_constructor (constructor_value);
  if (constructor_message_p != ECMA_IS_VALID_CONSTRUCTOR)
  {
    completion_value = ecma_raise_type_error (constructor_message_p);
  }
  else
  {
    ecma_object_t *constructor_obj_p = ecma_get_object_from_value (constructor_value);

    completion_value = ecma_op_function_construct (constructor_obj_p,
                                                   constructor_obj_p,
                                                   stack_top_p,
                                                   arguments_list_len);
  }

  /* Free registers. */
  for (uint32_t i = 0; i < arguments_list_len; i++)
  {
    ecma_fast_free_value (stack_top_p[i]);
  }

  if (JERRY_UNLIKELY (ECMA_IS_VALUE_ERROR (completion_value)))
  {
#if JERRY_DEBUGGER
    JERRY_CONTEXT (debugger_exception_byte_code_p) = frame_ctx_p->byte_code_p;
#endif /* JERRY_DEBUGGER */
    frame_ctx_p->byte_code_p = (uint8_t *) vm_error_byte_code_p;
  }
  else
  {
    ecma_free_value (stack_top_p[-1]);
    frame_ctx_p->byte_code_p = byte_code_p;
    stack_top_p[-1] = completion_value;
  }

  frame_ctx_p->stack_top_p = stack_top_p;
} /* opfunc_construct */