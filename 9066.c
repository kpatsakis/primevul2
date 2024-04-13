vm_run_eval (ecma_compiled_code_t *bytecode_data_p, /**< byte-code data */
             uint32_t parse_opts) /**< ecma_parse_opts_t option bits */
{
  ecma_value_t this_binding;
  ecma_object_t *lex_env_p;

  /* ECMA-262 v5, 10.4.2 */
  if (parse_opts & ECMA_PARSE_DIRECT_EVAL)
  {
    this_binding = ecma_copy_value (JERRY_CONTEXT (vm_top_context_p)->this_binding);
    lex_env_p = JERRY_CONTEXT (vm_top_context_p)->lex_env_p;

#if JERRY_DEBUGGER
    uint32_t chain_index = parse_opts >> ECMA_PARSE_CHAIN_INDEX_SHIFT;
    parse_opts &= (1 << ECMA_PARSE_CHAIN_INDEX_SHIFT) - 1;

    while (chain_index != 0)
    {
      if (JERRY_UNLIKELY (lex_env_p->u2.outer_reference_cp == JMEM_CP_NULL))
      {
        ecma_bytecode_deref (bytecode_data_p);
        ecma_free_value (this_binding);
        return ecma_raise_range_error (ECMA_ERR_MSG ("Invalid scope chain index for eval"));
      }

      lex_env_p = ECMA_GET_NON_NULL_POINTER (ecma_object_t, lex_env_p->u2.outer_reference_cp);

      if ((ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_THIS_OBJECT_BOUND)
          || (ecma_get_lex_env_type (lex_env_p) == ECMA_LEXICAL_ENVIRONMENT_DECLARATIVE))
      {
        chain_index--;
      }
    }
#endif /* JERRY_DEBUGGER */
  }
  else
  {
#if JERRY_BUILTIN_REALMS
    ecma_object_t *global_obj_p = (ecma_object_t *) ecma_op_function_get_realm (bytecode_data_p);
    this_binding = ((ecma_global_object_t *) global_obj_p)->this_binding;
    ecma_ref_object (ecma_get_object_from_value (this_binding));
#else /* !JERRY_BUILTIN_REALMS */
    ecma_object_t *global_obj_p = ecma_builtin_get_global ();
    ecma_ref_object (global_obj_p);
    this_binding = ecma_make_object_value (global_obj_p);
#endif /* JERRY_BUILTIN_REALMS */
    lex_env_p = ecma_get_global_scope (global_obj_p);
  }

  ecma_ref_object (lex_env_p);

  if ((bytecode_data_p->status_flags & CBC_CODE_FLAGS_STRICT_MODE) != 0)
  {
    ecma_object_t *strict_lex_env_p = ecma_create_decl_lex_env (lex_env_p);

    ecma_deref_object (lex_env_p);
    lex_env_p = strict_lex_env_p;
  }

  if ((bytecode_data_p->status_flags & CBC_CODE_FLAGS_LEXICAL_BLOCK_NEEDED) != 0)
  {
    ecma_object_t *lex_block_p = ecma_create_decl_lex_env (lex_env_p);
    lex_block_p->type_flags_refs |= ECMA_OBJECT_FLAG_BLOCK;

    ecma_deref_object (lex_env_p);
    lex_env_p = lex_block_p;
  }

  vm_frame_ctx_shared_t shared;
  shared.bytecode_header_p = bytecode_data_p;
  shared.function_object_p = NULL;
  shared.status_flags = (parse_opts & ECMA_PARSE_DIRECT_EVAL) ? VM_FRAME_CTX_SHARED_DIRECT_EVAL : 0;

  ecma_value_t completion_value = vm_run (&shared, this_binding, lex_env_p);

  ecma_deref_object (lex_env_p);
  ecma_free_value (this_binding);

#if JERRY_SNAPSHOT_EXEC
  if (!(bytecode_data_p->status_flags & CBC_CODE_FLAGS_STATIC_FUNCTION))
  {
    ecma_bytecode_deref (bytecode_data_p);
  }
#else /* !JERRY_SNAPSHOT_EXEC */
  ecma_bytecode_deref (bytecode_data_p);
#endif /* JERRY_SNAPSHOT_EXEC */

  return completion_value;
} /* vm_run_eval */