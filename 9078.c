vm_super_call (vm_frame_ctx_t *frame_ctx_p) /**< frame context */
{
  JERRY_ASSERT (frame_ctx_p->call_operation == VM_EXEC_SUPER_CALL);
  JERRY_ASSERT (frame_ctx_p->byte_code_p[0] == CBC_EXT_OPCODE);

  const uint8_t *byte_code_p = frame_ctx_p->byte_code_p + 3;
  uint8_t opcode = byte_code_p[-2];
  uint32_t arguments_list_len;

  bool spread_arguments = opcode >= CBC_EXT_SPREAD_SUPER_CALL;

  ecma_collection_t *collection_p = NULL;
  ecma_value_t *arguments_p;

  if (spread_arguments)
  {
    ecma_value_t collection = *(--frame_ctx_p->stack_top_p);
    collection_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_collection_t, collection);
    arguments_p = collection_p->buffer_p;
    arguments_list_len = collection_p->item_count;
  }
  else
  {
    arguments_list_len = byte_code_p[-1];
    arguments_p = frame_ctx_p->stack_top_p;
  }

  ecma_value_t func_value = *(--frame_ctx_p->stack_top_p);
  ecma_value_t completion_value;

  ecma_environment_record_t *environment_record_p = ecma_op_get_environment_record (frame_ctx_p->lex_env_p);

  if (!ecma_is_constructor (func_value))
  {
    completion_value = ecma_raise_type_error (ECMA_ERR_MSG ("Value for class heritage is not a constructor"));
  }
  else
  {
    ecma_object_t *func_obj_p = ecma_get_object_from_value (func_value);
    completion_value = ecma_op_function_construct (func_obj_p,
                                                   JERRY_CONTEXT (current_new_target_p),
                                                   arguments_p,
                                                   arguments_list_len);

    if (!ECMA_IS_VALUE_ERROR (completion_value) && ecma_op_this_binding_is_initialized (environment_record_p))
    {
      ecma_free_value (completion_value);
      completion_value = ecma_raise_reference_error (ECMA_ERR_MSG ("Super constructor may only be called once"));
    }
  }

  /* Free registers. */
  for (uint32_t i = 0; i < arguments_list_len; i++)
  {
    ecma_fast_free_value (arguments_p[i]);
  }

  if (collection_p != NULL)
  {
    ecma_collection_destroy (collection_p);
  }

  if (ecma_is_value_object (completion_value))
  {
    ecma_value_t fields_value = opfunc_init_class_fields (vm_get_class_function (frame_ctx_p), completion_value);

    if (ECMA_IS_VALUE_ERROR (fields_value))
    {
      ecma_free_value (completion_value);
      completion_value = ECMA_VALUE_ERROR;
    }
  }

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
    ecma_op_bind_this_value (environment_record_p, completion_value);
    frame_ctx_p->this_binding = completion_value;

    frame_ctx_p->byte_code_p = byte_code_p;
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
  }
} /* vm_super_call */