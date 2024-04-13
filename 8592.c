njs_string_slice_args(njs_vm_t *vm, njs_slice_prop_t *slice, njs_value_t *args,
    njs_uint_t nargs)
{
    int64_t      start, end, length;
    njs_int_t    ret;
    njs_value_t  *value;

    length = slice->string_length;

    value = njs_arg(args, nargs, 1);

    if (njs_slow_path(!njs_is_number(value))) {
        ret = njs_value_to_integer(vm, value, &start);
        if (njs_slow_path(ret != NJS_OK)) {
            return ret;
        }

    } else {
        start = njs_number_to_integer(njs_number(value));
    }

    if (start < 0) {
        start += length;

        if (start < 0) {
            start = 0;
        }
    }

    if (start >= length) {
        start = 0;
        length = 0;

    } else {
        value = njs_arg(args, nargs, 2);

        if (njs_slow_path(!njs_is_number(value))) {
            if (njs_is_defined(value)) {
                ret = njs_value_to_integer(vm, value, &end);
                if (njs_slow_path(ret != NJS_OK)) {
                    return ret;
                }

            } else {
                end = length;
            }

        } else {
            end = njs_number_to_integer(njs_number(value));
        }

        if (end < 0) {
            end += length;
        }

        if (length >= end) {
            length = end - start;

            if (length < 0) {
                start = 0;
                length = 0;
            }

        } else {
            length -= start;
        }
    }

    slice->start = start;
    slice->length = length;

    return NJS_OK;
}