vm_run (vm_frame_ctx_shared_t *shared_p, /**< shared data */
        ecma_value_t this_binding_value, /**< value of 'ThisBinding' */
        ecma_object_t *lex_env_p) /**< lexical environment to use */
{
  const ecma_compiled_code_t *bytecode_header_p = shared_p->bytecode_header_p;
  vm_frame_ctx_t *frame_ctx_p;
  size_t frame_size;

  if (bytecode_header_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
  {
    cbc_uint16_arguments_t *args_p = (cbc_uint16_arguments_t *) bytecode_header_p;
    frame_size = (size_t) (args_p->register_end + args_p->stack_limit);
  }
  else
  {
    cbc_uint8_arguments_t *args_p = (cbc_uint8_arguments_t *) bytecode_header_p;
    frame_size = (size_t) (args_p->register_end + args_p->stack_limit);
  }

  JERRY_VLA (ecma_value_t, stack, frame_size + (sizeof (vm_frame_ctx_t) / sizeof (ecma_value_t)));

  frame_ctx_p = (vm_frame_ctx_t *) stack;

  frame_ctx_p->shared_p = shared_p;
  frame_ctx_p->lex_env_p = lex_env_p;
  frame_ctx_p->this_binding = this_binding_value;

  vm_init_exec (frame_ctx_p);
  return vm_execute (frame_ctx_p);
} /* vm_run */