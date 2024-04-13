njs_string_prototype_concat(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    u_char             *p, *start;
    uint64_t           size, length, mask;
    njs_int_t          ret;
    njs_uint_t         i;
    njs_string_prop_t  string;

    if (njs_is_null_or_undefined(&args[0])) {
        njs_type_error(vm, "\"this\" argument is null or undefined");
        return NJS_ERROR;
    }

    for (i = 0; i < nargs; i++) {
        if (!njs_is_string(&args[i])) {
            ret = njs_value_to_string(vm, &args[i], &args[i]);
            if (ret != NJS_OK) {
                return ret;
            }
        }
    }

    if (nargs == 1) {
        njs_string_copy(&vm->retval, &args[0]);
        return NJS_OK;
    }

    size = 0;
    length = 0;
    mask = -1;

    for (i = 0; i < nargs; i++) {
        (void) njs_string_prop(&string, &args[i]);

        size += string.size;
        length += string.length;

        if (njs_is_byte_string(&string)) {
            mask = 0;
        }
    }

    length &= mask;

    start = njs_string_alloc(vm, &vm->retval, size, length);
    if (njs_slow_path(start == NULL)) {
        return NJS_ERROR;
    }

    p = start;

    for (i = 0; i < nargs; i++) {
        (void) njs_string_prop(&string, &args[i]);

        p = memcpy(p, string.start, string.size);
        p += string.size;
    }

    return NJS_OK;
}