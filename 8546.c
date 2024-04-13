njs_string_prototype_split(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    size_t             size;
    uint32_t           limit;
    njs_int_t          ret;
    njs_utf8_t         utf8;
    njs_bool_t         undefined;
    njs_value_t        *this, *separator, *value;
    njs_value_t        separator_lvalue, limit_lvalue, splitter;
    njs_array_t        *array;
    const u_char       *p, *start, *next, *last, *end;
    njs_string_prop_t  string, split;
    njs_value_t        arguments[3];

    static const njs_value_t  split_key =
                                        njs_wellknown_symbol(NJS_SYMBOL_SPLIT);

    this = njs_argument(args, 0);

    if (njs_slow_path(njs_is_null_or_undefined(this))) {
        njs_type_error(vm, "cannot convert \"%s\"to object",
                       njs_type_string(this->type));
        return NJS_ERROR;
    }

    separator = njs_lvalue_arg(&separator_lvalue, args, nargs, 1);
    value = njs_lvalue_arg(&limit_lvalue, args, nargs, 2);

    if (!njs_is_null_or_undefined(separator)) {
        ret = njs_value_method(vm, separator, njs_value_arg(&split_key),
                               &splitter);
        if (njs_slow_path(ret != NJS_OK)) {
            return ret;
        }

        if (njs_is_defined(&splitter)) {
            arguments[0] = *this;
            arguments[1] = *value;

            return njs_function_call(vm, njs_function(&splitter), separator,
                                     arguments, 2, &vm->retval);
        }
    }

    ret = njs_value_to_string(vm, this, this);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    array = njs_array_alloc(vm, 0, 0, NJS_ARRAY_SPARE);
    if (njs_slow_path(array == NULL)) {
        return NJS_ERROR;
    }

    limit = UINT32_MAX;

    if (njs_is_defined(value)) {
        ret = njs_value_to_uint32(vm, value, &limit);
        if (njs_slow_path(ret != NJS_OK)) {
            return ret;
        }
    }

    undefined = njs_is_undefined(separator);

    ret = njs_value_to_string(vm, separator, separator);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    if (njs_slow_path(limit == 0)) {
        goto done;
    }

    if (njs_slow_path(undefined)) {
        goto single;
    }

    (void) njs_string_prop(&string, this);
    (void) njs_string_prop(&split, separator);

    if (njs_slow_path(string.size == 0)) {
        if (split.size != 0) {
            goto single;
        }

        goto done;
    }

    utf8 = NJS_STRING_BYTE;

    if (string.length != 0) {
        utf8 = NJS_STRING_ASCII;

        if (string.length != string.size) {
            utf8 = NJS_STRING_UTF8;
        }
    }

    start = string.start;
    end = string.start + string.size;
    last = end - split.size;

    do {

        for (p = start; p <= last; p++) {
            if (memcmp(p, split.start, split.size) == 0) {
                goto found;
            }
        }

        p = end;

found:

        next = p + split.size;

        /* Empty split string. */

        if (p == next) {
            p = (utf8 != NJS_STRING_BYTE) ? njs_utf8_next(p, end)
                                          : p + 1;
            next = p;
        }

        size = p - start;

        ret = njs_string_split_part_add(vm, array, utf8, start, size);
        if (njs_slow_path(ret != NJS_OK)) {
            return ret;
        }

        start = next;
        limit--;

    } while (limit != 0 && p < end);

    goto done;

single:

    value = njs_array_push(vm, array);
    if (njs_slow_path(value == NULL)) {
        return NJS_ERROR;
    }

    *value = *this;

done:

    njs_set_array(&vm->retval, array);

    return NJS_OK;
}