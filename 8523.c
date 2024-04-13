njs_string_constructor(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    njs_int_t           ret;
    njs_value_t         *value;
    njs_object_value_t  *object;

    if (nargs == 1) {
        value = njs_value_arg(&njs_string_empty);

    } else {
        value = &args[1];

        if (njs_slow_path(!njs_is_string(value))) {
            if (!vm->top_frame->ctor && njs_is_symbol(value)) {
                return njs_symbol_descriptive_string(vm, &vm->retval, value);
            }

            ret = njs_value_to_string(vm, value, value);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }
        }
    }

    if (vm->top_frame->ctor) {
        object = njs_object_value_alloc(vm, NJS_OBJ_TYPE_STRING, 0, value);
        if (njs_slow_path(object == NULL)) {
            return NJS_ERROR;
        }

        njs_set_object_value(&vm->retval, object);

    } else {
        vm->retval = *value;
    }

    return NJS_OK;
}