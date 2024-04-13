njs_string_prototype_includes(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    int64_t            index, length, search_length;
    njs_int_t          ret;
    njs_value_t        *value;
    const u_char       *p, *end;
    const njs_value_t  *retval;
    njs_string_prop_t  string, search;

    ret = njs_string_object_validate(vm, njs_argument(args, 0));
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    retval = &njs_value_true;

    if (nargs > 1) {
        value = njs_argument(args, 1);

        if (njs_slow_path(!njs_is_string(value))) {
            ret = njs_value_to_string(vm, value, value);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }
        }

        search_length = njs_string_prop(&search, value);

        if (nargs > 2) {
            value = njs_argument(args, 2);

            if (njs_slow_path(!njs_is_number(value))) {
                ret = njs_value_to_integer(vm, value, &index);
                if (njs_slow_path(ret != NJS_OK)) {
                    return ret;
                }

            } else {
                index = njs_number_to_integer(njs_number(value));
            }

            if (index < 0) {
                index = 0;
            }

        } else {
            index = 0;
        }

        if (search_length == 0) {
            goto done;
        }

        length = njs_string_prop(&string, &args[0]);

        if (length - index >= search_length) {
            end = string.start + string.size;

            if (string.size == (size_t) length) {
                /* Byte or ASCII string. */
                p = string.start + index;

            } else {
                /* UTF-8 string. */
                p = njs_string_offset(string.start, end, index);
            }

            end -= search.size - 1;

            while (p < end) {
                if (memcmp(p, search.start, search.size) == 0) {
                    goto done;
                }

                p++;
            }
        }
    }

    retval = &njs_value_false;

done:

    vm->retval = *retval;

    return NJS_OK;
}