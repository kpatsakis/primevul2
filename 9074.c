opfunc_call (vm_frame_ctx_t *frame_ctx_p) /**< frame context */
{
  const uint8_t *byte_code_p = frame_ctx_p->byte_code_p + 1;
  uint8_t opcode = byte_code_p[-1];
  uint32_t arguments_list_len;

  if (opcode >= CBC_CALL0)
  {
    arguments_list_len = (unsigned int) ((opcode - CBC_CALL0) / 6);
  }
  else
  {
    arguments_list_len = *byte_code_p++;
  }

  bool is_call_prop = ((opcode - CBC_CALL) % 6) >= 3;

  ecma_value_t *stack_top_p = frame_ctx_p->stack_top_p - arguments_list_len;
  ecma_value_t this_value = is_call_prop ? stack_top_p[-3] : ECMA_VALUE_UNDEFINED;
  ecma_value_t func_value = stack_top_p[-1];
  ecma_value_t completion_value;

  if (!ecma_is_value_object (func_value)
      || !ecma_op_object_is_callable (ecma_get_object_from_value (func_value)))
  {
    completion_value = ecma_raise_type_error (ECMA_ERR_MSG (ecma_error_expected_a_function));
  }
  else
  {
    ecma_object_t *func_obj_p = ecma_get_object_from_value (func_value);

    completion_value = ecma_op_function_call (func_obj_p,
                                              this_value,
                                              stack_top_p,
                                              arguments_list_len);
  }

  JERRY_CONTEXT (status_flags) &= (uint32_t) ~ECMA_STATUS_DIRECT_EVAL;

  /* Free registers. */
  for (uint32_t i = 0; i < arguments_list_len; i++)
  {
    ecma_fast_free_value (stack_top_p[i]);
  }

  if (is_call_prop)
  {
    ecma_free_value (*(--stack_top_p));
    ecma_free_value (*(--stack_top_p));
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
    frame_ctx_p->byte_code_p = byte_code_p;
    ecma_free_value (*(--stack_top_p));
    uint32_t opcode_data = vm_decode_table[opcode];

    if (!(opcode_data & (VM_OC_PUT_STACK | VM_OC_PUT_BLOCK)))
    {
      ecma_fast_free_value (completion_value);
    }
    else if (opcode_data & VM_OC_PUT_STACK)
    {
      *stack_top_p++ = completion_value;
    }
    else
    {
      ecma_fast_free_value (VM_GET_REGISTER (frame_ctx_p, 0));
      VM_GET_REGISTERS (frame_ctx_p)[0] = completion_value;
    }
  }

  frame_ctx_p->stack_top_p = stack_top_p;
} /* opfunc_call */