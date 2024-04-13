njs_string_prototype_substring(njs_vm_t *vm, njs_value_t *args,
    njs_uint_t nargs, njs_index_t unused)
{
    int64_t            start, end, length;
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
        value = njs_argument(args, 1);

        if (njs_slow_path(!njs_is_number(value))) {
            ret = njs_value_to_integer(vm, value, &start);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }

        } else {
            start = njs_number_to_integer(njs_number(value));
        }

        if (start < 0) {
            start = 0;

        } else if (start > length) {
            start = length;
        }

        end = length;

        if (nargs > 2) {
            value = njs_arg(args, nargs, 2);

            if (njs_slow_path(!njs_is_number(value))) {
                ret = njs_value_to_integer(vm, value, &end);
                if (njs_slow_path(ret != NJS_OK)) {
                    return ret;
                }

            } else {
                end = njs_number_to_integer(njs_number(value));
            }

            if (end < 0) {
                end = 0;

            } else if (end >= length) {
                end = length;
            }
        }

        length = end - start;

        if (length < 0) {
            length = -length;
            start = end;
        }
    }

    slice.start = start;
    slice.length = length;

    return njs_string_slice(vm, &vm->retval, &string, &slice);
}