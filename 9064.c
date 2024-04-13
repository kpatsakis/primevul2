vm_execute (vm_frame_ctx_t *frame_ctx_p) /**< frame context */
{
  while (true)
  {
    ecma_value_t completion_value = vm_loop (frame_ctx_p);

    switch (frame_ctx_p->call_operation)
    {
      case VM_EXEC_CALL:
      {
        opfunc_call (frame_ctx_p);
        break;
      }
#if JERRY_ESNEXT
      case VM_EXEC_SUPER_CALL:
      {
        vm_super_call (frame_ctx_p);
        break;
      }
      case VM_EXEC_SPREAD_OP:
      {
        vm_spread_operation (frame_ctx_p);
        break;
      }
      case VM_EXEC_RETURN:
      {
        return completion_value;
      }
#endif /* JERRY_ESNEXT */
      case VM_EXEC_CONSTRUCT:
      {
        opfunc_construct (frame_ctx_p);
        break;
      }
      default:
      {
        JERRY_ASSERT (frame_ctx_p->call_operation == VM_NO_EXEC_OP);

        const ecma_compiled_code_t *bytecode_header_p = frame_ctx_p->shared_p->bytecode_header_p;
        uint32_t register_end;

        if (bytecode_header_p->status_flags & CBC_CODE_FLAGS_UINT16_ARGUMENTS)
        {
          register_end = ((cbc_uint16_arguments_t *) bytecode_header_p)->register_end;
        }
        else
        {
          register_end = ((cbc_uint8_arguments_t *) bytecode_header_p)->register_end;
        }

        /* Free arguments and registers */
        ecma_value_t *registers_p = VM_GET_REGISTERS (frame_ctx_p);
        for (uint32_t i = 0; i < register_end; i++)
        {
          ecma_fast_free_value (registers_p[i]);
        }

#if JERRY_DEBUGGER
        if (JERRY_CONTEXT (debugger_stop_context) == JERRY_CONTEXT (vm_top_context_p))
        {
          /* The engine will stop when the next breakpoint is reached. */
          JERRY_ASSERT (JERRY_CONTEXT (debugger_flags) & JERRY_DEBUGGER_VM_STOP);
          JERRY_CONTEXT (debugger_stop_context) = NULL;
        }
#endif /* JERRY_DEBUGGER */

        JERRY_CONTEXT (vm_top_context_p) = frame_ctx_p->prev_context_p;
        return completion_value;
      }
    }
  }
} /* vm_execute */