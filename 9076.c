vm_construct_literal_object (vm_frame_ctx_t *frame_ctx_p, /**< frame context */
                             ecma_value_t lit_value) /**< literal */
{
  ecma_compiled_code_t *bytecode_p;

#if JERRY_SNAPSHOT_EXEC
  if (JERRY_LIKELY (!(frame_ctx_p->shared_p->bytecode_header_p->status_flags & CBC_CODE_FLAGS_STATIC_FUNCTION)))
  {
#endif /* JERRY_SNAPSHOT_EXEC */
    bytecode_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_compiled_code_t,
                                                  lit_value);
#if JERRY_SNAPSHOT_EXEC
  }
  else
  {
    uint8_t *byte_p = ((uint8_t *) frame_ctx_p->shared_p->bytecode_header_p) + lit_value;
    bytecode_p = (ecma_compiled_code_t *) byte_p;
  }
#endif /* JERRY_SNAPSHOT_EXEC */

#if JERRY_BUILTIN_REGEXP
  if (JERRY_UNLIKELY (!CBC_IS_FUNCTION (bytecode_p->status_flags)))
  {
    ecma_object_t *regexp_obj_p = ecma_op_regexp_alloc (NULL);

    if (JERRY_UNLIKELY (regexp_obj_p == NULL))
    {
      return ECMA_VALUE_ERROR;
    }

    return ecma_op_create_regexp_from_bytecode (regexp_obj_p, (re_compiled_code_t *) bytecode_p);
  }
#else /* !JERRY_BUILTIN_REGEXP */
  JERRY_ASSERT (CBC_IS_FUNCTION (bytecode_p->status_flags));
#endif /* JERRY_BUILTIN_REGEXP */

  ecma_object_t *func_obj_p;

#if JERRY_ESNEXT
  if (JERRY_UNLIKELY (CBC_FUNCTION_IS_ARROW (bytecode_p->status_flags)))
  {
    func_obj_p = ecma_op_create_arrow_function_object (frame_ctx_p->lex_env_p,
                                                       bytecode_p,
                                                       frame_ctx_p->this_binding);
  }
  else
  {
    func_obj_p = ecma_op_create_any_function_object (frame_ctx_p->lex_env_p, bytecode_p);
  }
#else /* !JERRY_ESNEXT */
  func_obj_p = ecma_op_create_simple_function_object (frame_ctx_p->lex_env_p, bytecode_p);
#endif /* JERRY_ESNEXT */

  return ecma_make_object_value (func_obj_p);
} /* vm_construct_literal_object */