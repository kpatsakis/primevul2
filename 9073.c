vm_run_global (const ecma_compiled_code_t *bytecode_p, /**< pointer to bytecode to run */
               ecma_object_t *function_object_p) /**< function object if available */
{
#if JERRY_BUILTIN_REALMS
  ecma_object_t *global_obj_p = (ecma_object_t *) ecma_op_function_get_realm (bytecode_p);
#else /* !JERRY_BUILTIN_REALMS */
  ecma_object_t *global_obj_p = ecma_builtin_get_global ();
#endif /* JERRY_BUILTIN_REALMS */

#if JERRY_ESNEXT
  if (bytecode_p->status_flags & CBC_CODE_FLAGS_LEXICAL_BLOCK_NEEDED)
  {
    ecma_create_global_lexical_block (global_obj_p);
  }
#endif /* JERRY_ESNEXT */

  ecma_object_t *const global_scope_p = ecma_get_global_scope (global_obj_p);

  vm_frame_ctx_shared_t shared;
  shared.bytecode_header_p = bytecode_p;
  shared.function_object_p = function_object_p;
  shared.status_flags = 0;

#if JERRY_BUILTIN_REALMS
  ecma_value_t this_binding = ((ecma_global_object_t *) global_obj_p)->this_binding;

  ecma_global_object_t *saved_global_object_p = JERRY_CONTEXT (global_object_p);
  JERRY_CONTEXT (global_object_p) = (ecma_global_object_t *) global_obj_p;
#else /* !JERRY_BUILTIN_REALMS */
  ecma_value_t this_binding = ecma_make_object_value (global_obj_p);
#endif /* JERRY_BUILTIN_REALMS */

  ecma_value_t result = vm_run (&shared, this_binding, global_scope_p);

#if JERRY_BUILTIN_REALMS
  JERRY_CONTEXT (global_object_p) = saved_global_object_p;
#endif /* JERRY_BUILTIN_REALMS */

  return result;
} /* vm_run_global */