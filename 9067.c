vm_init_exec (vm_frame_ctx_t *frame_ctx_p) /**< frame context */
{
  vm_frame_ctx_shared_t *shared_p = frame_ctx_p->shared_p;
  const ecma_compiled_code_t *bytecode_header_p = shared_p->bytecode_header_p;

  frame_ctx_p->prev_context_p = JERRY_CONTEXT (vm_top_context_p);
  frame_ctx_p->context_depth = 0;
  frame_ctx_p->status_flags = (uint8_t) ((shared_p->status_flags & VM_FRAME_CTX_DIRECT_EVAL)
                                         | (bytecode_header_p->status_flags & VM_FRAME_CTX_IS_STRICT));

  uint16_t argument_end, register_end;
  ecma_value_t *literal_p;

  if (bytecode_header_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
  {
    cbc_uint16_arguments_t *args_p = (cbc_uint16_arguments_t *) bytecode_header_p;

    argument_end = args_p->argument_end;
    register_end = args_p->register_end;

    literal_p = (ecma_value_t *) (args_p + 1);
    literal_p -= register_end;
    frame_ctx_p->literal_start_p = literal_p;
    literal_p += args_p->literal_end;
  }
  else
  {
    cbc_uint8_arguments_t *args_p = (cbc_uint8_arguments_t *) bytecode_header_p;

    argument_end = args_p->argument_end;
    register_end = args_p->register_end;

    literal_p = (ecma_value_t *) (args_p + 1);
    literal_p -= register_end;
    frame_ctx_p->literal_start_p = literal_p;
    literal_p += args_p->literal_end;
  }

  frame_ctx_p->byte_code_p = (uint8_t *) literal_p;
  frame_ctx_p->byte_code_start_p = (uint8_t *) literal_p;
  frame_ctx_p->stack_top_p = VM_GET_REGISTERS (frame_ctx_p) + register_end;

  uint32_t arg_list_len = 0;

  if (argument_end > 0)
  {
    JERRY_ASSERT (shared_p->status_flags & VM_FRAME_CTX_SHARED_HAS_ARG_LIST);

    const ecma_value_t *arg_list_p = ((vm_frame_ctx_shared_args_t *) shared_p)->arg_list_p;
    arg_list_len = ((vm_frame_ctx_shared_args_t *) shared_p)->arg_list_len;

    if (arg_list_len > argument_end)
    {
      arg_list_len = argument_end;
    }

    for (uint32_t i = 0; i < arg_list_len; i++)
    {
      VM_GET_REGISTER (frame_ctx_p, i) = ecma_fast_copy_value (arg_list_p[i]);
    }
  }

  /* The arg_list_len contains the end of the copied arguments.
   * Fill everything else with undefined. */
  if (register_end > arg_list_len)
  {
    ecma_value_t *stack_p = VM_GET_REGISTERS (frame_ctx_p) + arg_list_len;

    for (uint32_t i = arg_list_len; i < register_end; i++)
    {
      *stack_p++ = ECMA_VALUE_UNDEFINED;
    }
  }

  JERRY_CONTEXT (status_flags) &= (uint32_t) ~ECMA_STATUS_DIRECT_EVAL;
  JERRY_CONTEXT (vm_top_context_p) = frame_ctx_p;
} /* vm_init_exec */