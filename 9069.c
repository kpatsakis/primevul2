vm_get_class_function (vm_frame_ctx_t *frame_ctx_p) /**< frame context */
{
  JERRY_ASSERT (frame_ctx_p != NULL);

  if (frame_ctx_p->shared_p->status_flags & VM_FRAME_CTX_SHARED_NON_ARROW_FUNC)
  {
    return frame_ctx_p->shared_p->function_object_p;
  }

  ecma_environment_record_t *environment_record_p = ecma_op_get_environment_record (frame_ctx_p->lex_env_p);

  JERRY_ASSERT (environment_record_p != NULL);
  return ecma_get_object_from_value (environment_record_p->function_object);
} /* vm_get_class_function */