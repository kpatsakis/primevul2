vm_init_module_scope (ecma_module_t *module_p) /**< module without scope */
{
  ecma_object_t *global_object_p;
#if JERRY_BUILTIN_REALMS
  global_object_p = (ecma_object_t *) ecma_op_function_get_realm (module_p->u.compiled_code_p);
#else /* !JERRY_BUILTIN_REALMS */
  global_object_p = ecma_builtin_get_global ();
#endif /* JERRY_BUILTIN_REALMS */

  ecma_object_t *scope_p = ecma_create_lex_env_class (ecma_get_global_environment (global_object_p),
                                                      sizeof (ecma_lexical_environment_class_t));
  const ecma_compiled_code_t *compiled_code_p = module_p->u.compiled_code_p;
  ecma_value_t *literal_start_p;
  uint8_t *byte_code_p;
  uint16_t encoding_limit;
  uint16_t encoding_delta;

  ((ecma_lexical_environment_class_t *) scope_p)->module_p = (ecma_object_t *) module_p;

  module_p->scope_p = scope_p;
  ecma_deref_object (scope_p);

  if (compiled_code_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
  {
    cbc_uint16_arguments_t *args_p = (cbc_uint16_arguments_t *) compiled_code_p;

    literal_start_p = (ecma_value_t *) (args_p + 1);
    literal_start_p -= args_p->register_end;
    byte_code_p = (uint8_t *) (literal_start_p + args_p->literal_end);
  }
  else
  {
    cbc_uint8_arguments_t *args_p = (cbc_uint8_arguments_t *) compiled_code_p;

    literal_start_p = (ecma_value_t *) (args_p + 1);
    literal_start_p -= args_p->register_end;
    byte_code_p = (uint8_t *) (literal_start_p + args_p->literal_end);
  }

  /* Prepare for byte code execution. */
  if (!(compiled_code_p->status_flags & CBC_CODE_FLAGS_FULL_LITERAL_ENCODING))
  {
    encoding_limit = CBC_SMALL_LITERAL_ENCODING_LIMIT;
    encoding_delta = CBC_SMALL_LITERAL_ENCODING_DELTA;
  }
  else
  {
    encoding_limit = CBC_FULL_LITERAL_ENCODING_LIMIT;
    encoding_delta = CBC_FULL_LITERAL_ENCODING_DELTA;
  }

  JERRY_ASSERT (*byte_code_p >= CBC_JUMP_FORWARD && *byte_code_p <= CBC_JUMP_FORWARD_3);

  byte_code_p += 1 + CBC_BRANCH_OFFSET_LENGTH (*byte_code_p);

  while (true)
  {
    uint8_t opcode = *byte_code_p++;

    switch (opcode)
    {
      case CBC_CREATE_VAR:
      case CBC_CREATE_LET:
      case CBC_CREATE_CONST:
      {
        uint32_t literal_index;

        READ_LITERAL_INDEX (literal_index);

        ecma_string_t *name_p = ecma_get_string_from_value (literal_start_p[literal_index]);

        JERRY_ASSERT (ecma_find_named_property (scope_p, name_p) == NULL);

        uint8_t prop_attributes = ECMA_PROPERTY_FLAG_WRITABLE;

        if (opcode == CBC_CREATE_LET)
        {
          prop_attributes = ECMA_PROPERTY_ENUMERABLE_WRITABLE;
        }
        else if (opcode == CBC_CREATE_CONST)
        {
          prop_attributes = ECMA_PROPERTY_FLAG_ENUMERABLE;
        }

        ecma_property_value_t *property_value_p;
        property_value_p = ecma_create_named_data_property (scope_p, name_p, prop_attributes, NULL);

        if (opcode != CBC_CREATE_VAR)
        {
          property_value_p->value = ECMA_VALUE_UNINITIALIZED;
        }
        break;
      }
      case CBC_INIT_ARG_OR_FUNC:
      {
        uint32_t literal_index;

        READ_LITERAL_INDEX (literal_index);

        ecma_compiled_code_t *function_bytecode_p;
#if JERRY_SNAPSHOT_EXEC
        if (JERRY_LIKELY (!(compiled_code_p->status_flags & CBC_CODE_FLAGS_STATIC_FUNCTION)))
        {
#endif /* JERRY_SNAPSHOT_EXEC */
          function_bytecode_p = ECMA_GET_INTERNAL_VALUE_POINTER (ecma_compiled_code_t,
                                                                 literal_start_p[literal_index]);
#if JERRY_SNAPSHOT_EXEC
        }
        else
        {
          uint8_t *byte_p = ((uint8_t *) compiled_code_p) + literal_start_p[literal_index];
          function_bytecode_p = (ecma_compiled_code_t *) byte_p;
        }
#endif /* JERRY_SNAPSHOT_EXEC */

        JERRY_ASSERT (CBC_IS_FUNCTION (function_bytecode_p->status_flags));

        ecma_object_t *function_obj_p;

        if (JERRY_UNLIKELY (CBC_FUNCTION_IS_ARROW (function_bytecode_p->status_flags)))
        {
          function_obj_p = ecma_op_create_arrow_function_object (scope_p,
                                                                 function_bytecode_p,
                                                                 ECMA_VALUE_UNDEFINED);
        }
        else
        {
          function_obj_p = ecma_op_create_any_function_object (scope_p, function_bytecode_p);
        }

        READ_LITERAL_INDEX (literal_index);
        ecma_string_t *name_p = ecma_get_string_from_value (literal_start_p[literal_index]);

        JERRY_ASSERT (ecma_find_named_property (scope_p, name_p) == NULL);

        ecma_property_value_t *property_value_p;
        property_value_p = ecma_create_named_data_property (scope_p,
                                                            name_p,
                                                            ECMA_PROPERTY_FLAG_WRITABLE,
                                                            NULL);

        JERRY_ASSERT (property_value_p->value == ECMA_VALUE_UNDEFINED);
        property_value_p->value = ecma_make_object_value (function_obj_p);
        ecma_deref_object (function_obj_p);
        break;
      }
      default:
      {
        JERRY_ASSERT (opcode == CBC_RETURN_FUNCTION_END);
        return ECMA_VALUE_EMPTY;
      }
    }
  }
} /* vm_init_module_scope */