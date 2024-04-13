njs_string_prototype_pad(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t pad_start)
{
    u_char             *p, *start;
    size_t             padding, trunc, new_size;
    int64_t            length, new_length;
    uint32_t           n, pad_length;
    njs_int_t          ret;
    njs_value_t        *value, *pad;
    const u_char       *end;
    njs_string_prop_t  string, pad_string;

    static const njs_value_t  string_space = njs_string(" ");

    ret = njs_string_object_validate(vm, njs_argument(args, 0));
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    length = njs_string_prop(&string, njs_argument(args, 0));

    new_length = 0;

    if (nargs > 1) {
        value = njs_argument(args, 1);

        if (njs_slow_path(!njs_is_number(value))) {
            ret = njs_value_to_integer(vm, value, &new_length);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }

        } else {
            new_length = njs_number_to_integer(njs_number(value));
        }
    }

    if (new_length <= length) {
        vm->retval = args[0];
        return NJS_OK;
    }

    if (njs_slow_path(new_length >= NJS_STRING_MAX_LENGTH)) {
        njs_range_error(vm, NULL);
        return NJS_ERROR;
    }

    padding = new_length - length;

    /* GCC and Clang complain about uninitialized n and trunc. */
    n = 0;
    trunc = 0;

    pad = njs_arg(args, nargs, 2);

    if (njs_slow_path(!njs_is_string(pad))) {
        if (njs_is_undefined(pad)) {
            pad = njs_value_arg(&string_space);

        } else {
            ret = njs_value_to_string(vm, pad, pad);
            if (njs_slow_path(ret != NJS_OK)) {
                return NJS_ERROR;
            }
        }
    }

    pad_length = njs_string_prop(&pad_string, pad);

    if (pad_string.size == 0) {
        vm->retval = args[0];
        return NJS_OK;
    }

    if (pad_string.size > 1) {
        n = padding / pad_length;
        trunc = padding % pad_length;

        if (pad_string.size != (size_t) pad_length) {
            /* UTF-8 string. */
            end = pad_string.start + pad_string.size;
            end = njs_string_offset(pad_string.start, end, trunc);

            trunc = end - pad_string.start;
            padding = pad_string.size * n + trunc;
        }
    }

    new_size = string.size + padding;

    start = njs_string_alloc(vm, &vm->retval, new_size, new_length);
    if (njs_slow_path(start == NULL)) {
        return NJS_ERROR;
    }

    p = start;

    if (pad_start) {
        start += padding;

    } else {
        p += string.size;
    }

    memcpy(start, string.start, string.size);

    if (pad_string.size == 1) {
        njs_memset(p, pad_string.start[0], padding);

    } else {
        while (n != 0) {
            memcpy(p, pad_string.start, pad_string.size);
            p += pad_string.size;
            n--;
        }

        memcpy(p, pad_string.start, trunc);
    }

    return NJS_OK;
}