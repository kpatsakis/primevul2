vm_run_module (ecma_module_t *module_p) /**< module to be executed */
{
  const ecma_value_t module_init_result = ecma_module_initialize (module_p);

  if (ECMA_IS_VALUE_ERROR (module_init_result))
  {
    return module_init_result;
  }

  vm_frame_ctx_shared_t shared;
  shared.bytecode_header_p = module_p->u.compiled_code_p;
  shared.function_object_p = &module_p->header.object;
  shared.status_flags = 0;

  return vm_run (&shared, ECMA_VALUE_UNDEFINED, module_p->scope_p);
} /* vm_run_module */