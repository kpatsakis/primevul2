vm_spread_operation (vm_frame_ctx_t *frame_ctx_p) /**< frame context */
{
  JERRY_ASSERT (frame_ctx_p->byte_code_p[0] == CBC_EXT_OPCODE);

  uint8_t opcode = frame_ctx_p->byte_code_p[1];
  ecma_value_t completion_value;
  ecma_value_t collection = *(--frame_ctx_p->stack_top_p);

  ecma_collection_t *collection_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_collection_t, collection);
  ecma_value_t func_value = *(--frame_ctx_p->stack_top_p);
  bool is_call_prop = opcode >= CBC_EXT_SPREAD_CALL_PROP;

  if (frame_ctx_p->byte_code_p[1] == CBC_EXT_SPREAD_NEW)
  {
    const char *constructor_message_p = ecma_check_constructor (func_value);
    if (constructor_message_p != ECMA_IS_VALID_CONSTRUCTOR)
    {
      completion_value = ecma_raise_type_error (constructor_message_p);
    }
    else
    {
      ecma_object_t *constructor_obj_p = ecma_get_object_from_value (func_value);

      completion_value = ecma_op_function_construct (constructor_obj_p,
                                                     constructor_obj_p,
                                                     collection_p->buffer_p,
                                                     collection_p->item_count);
    }
  }
  else
  {
    ecma_value_t this_value = is_call_prop ? frame_ctx_p->stack_top_p[-2] : ECMA_VALUE_UNDEFINED;

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
                                                collection_p->buffer_p,
                                                collection_p->item_count);
    }

    if (is_call_prop)
    {
      ecma_free_value (*(--frame_ctx_p->stack_top_p));
      ecma_free_value (*(--frame_ctx_p->stack_top_p));
    }
  }

  ecma_collection_free (collection_p);
  ecma_free_value (func_value);

  if (JERRY_UNLIKELY (ECMA_IS_VALUE_ERROR (completion_value)))
  {
#if JERRY_DEBUGGER
    JERRY_CONTEXT (debugger_exception_byte_code_p) = frame_ctx_p->byte_code_p;
#endif /* JERRY_DEBUGGER */
    frame_ctx_p->byte_code_p = (uint8_t *) vm_error_byte_code_p;
  }
  else
  {
    uint32_t opcode_data = vm_decode_table[(CBC_END + 1) + opcode];

    if (!(opcode_data & (VM_OC_PUT_STACK | VM_OC_PUT_BLOCK)))
    {
      ecma_fast_free_value (completion_value);
    }
    else if (opcode_data & VM_OC_PUT_STACK)
    {
      *frame_ctx_p->stack_top_p++ = completion_value;
    }
    else
    {
      ecma_fast_free_value (VM_GET_REGISTER (frame_ctx_p, 0));
      VM_GET_REGISTERS (frame_ctx_p)[0] = completion_value;
    }

    /* EXT_OPCODE, SPREAD_OPCODE, BYTE_ARG */
    frame_ctx_p->byte_code_p += 3;
  }
} /* vm_spread_operation */