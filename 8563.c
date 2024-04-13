njs_string_prototype_substr(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    int64_t            start, length, n;
    njs_int_t          ret;
    njs_value_t        *value;
    njs_slice_prop_t   slice;
    njs_string_prop_t  string;

    ret = njs_string_object_validate(vm, njs_argument(args, 0));
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    length = njs_string_prop(&string, njs_argument(args, 0));

    slice.string_length = length;
    start = 0;

    if (nargs > 1) {
        value = njs_arg(args, nargs, 1);

        if (njs_slow_path(!njs_is_number(value))) {
            ret = njs_value_to_integer(vm, value, &start);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }

        } else {
            start = njs_number_to_integer(njs_number(value));
        }

        if (start < length) {
            if (start < 0) {
                start += length;

                if (start < 0) {
                    start = 0;
                }
            }

            length -= start;

            if (nargs > 2) {
                value = njs_arg(args, nargs, 2);

                if (njs_slow_path(!njs_is_number(value))) {
                    ret = njs_value_to_integer(vm, value, &n);
                    if (njs_slow_path(ret != NJS_OK)) {
                        return ret;
                    }

                } else {
                    n = njs_number_to_integer(njs_number(value));
                }

                if (n < 0) {
                    length = 0;

                } else if (n < length) {
                    length = n;
                }
            }

        } else {
            start = 0;
            length = 0;
        }
    }

    slice.start = start;
    slice.length = length;

    return njs_string_slice(vm, &vm->retval, &string, &slice);
}