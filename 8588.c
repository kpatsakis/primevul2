njs_string_prototype_starts_or_ends_with(njs_vm_t *vm, njs_value_t *args,
    njs_uint_t nargs, njs_index_t starts)
{
    int64_t            index, length, search_length;
    njs_int_t          ret;
    njs_value_t        *value, lvalue;
    const u_char       *p, *end;
    const njs_value_t  *retval;
    njs_string_prop_t  string, search;

    retval = &njs_value_true;

    ret = njs_string_object_validate(vm, njs_argument(args, 0));
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    value = njs_lvalue_arg(&lvalue, args, nargs, 1);

    if (njs_slow_path(!njs_is_string(value))) {
        ret = njs_value_to_string(vm, value, value);
        if (njs_slow_path(ret != NJS_OK)) {
            return ret;
        }
    }

    search_length = njs_string_prop(&search, value);

    value = njs_arg(args, nargs, 2);

    if (njs_slow_path(!njs_is_number(value))) {
        index = -1;

        if (!njs_is_undefined(value)) {
            ret = njs_value_to_integer(vm, value, &index);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }
        }

    } else {
        index = njs_number_to_integer(njs_number(value));
    }

    if (search_length == 0) {
        goto done;
    }

    if (nargs > 1) {
        length = njs_string_prop(&string, &args[0]);

        if (starts) {
            if (index < 0) {
                index = 0;
            }

            if (length - index < search_length) {
                goto small;
            }

        } else {
            if (index < 0 || index > length) {
                index = length;
            }

            index -= search_length;

            if (index < 0) {
                goto small;
            }
        }

        end = string.start + string.size;

        if (string.size == (size_t) length) {
            /* Byte or ASCII string. */
            p = string.start + index;

        } else {
            /* UTF-8 string. */
            p = njs_string_offset(string.start, end, index);
        }

        if ((size_t) (end - p) >= search.size
            && memcmp(p, search.start, search.size) == 0)
        {
            goto done;
        }
    }

small:

    retval = &njs_value_false;

done:

    vm->retval = *retval;

    return NJS_OK;
}