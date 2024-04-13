njs_string_prototype_last_index_of(njs_vm_t *vm, njs_value_t *args,
    njs_uint_t nargs, njs_index_t unused)
{
    double             pos;
    int64_t            index, start, length, search_length;
    njs_int_t          ret;
    njs_value_t        *this, *search, search_lvalue;
    const u_char       *p, *end;
    njs_string_prop_t  string, s;

    this = njs_argument(args, 0);

    if (njs_slow_path(njs_is_null_or_undefined(this))) {
        njs_type_error(vm, "cannot convert \"%s\"to object",
                       njs_type_string(this->type));
        return NJS_ERROR;
    }

    ret = njs_value_to_string(vm, this, this);
    if (njs_slow_path(ret != NJS_OK)) {
        return NJS_ERROR;
    }

    search = njs_lvalue_arg(&search_lvalue, args, nargs, 1);
    ret = njs_value_to_string(vm, search, search);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    ret = njs_value_to_number(vm, njs_arg(args, nargs, 2), &pos);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    if (!isnan(pos)) {
        start = njs_number_to_integer(pos);

    } else {
        start = INT64_MAX;
    }

    length = njs_string_prop(&string, this);

    start = njs_min(njs_max(start, 0), length);

    search_length = njs_string_prop(&s, search);

    index = length - search_length;

    if (index > start) {
        index = start;
    }

    end = string.start + string.size;

    if (string.size == (size_t) length) {
        /* Byte or ASCII string. */

        p = &string.start[index];

        if (p > end - s.size) {
            p = end - s.size;
        }

        for (; p >= string.start; p--) {
            if (memcmp(p, s.start, s.size) == 0) {
                index = p - string.start;
                goto done;
            }
        }

        index = -1;

    } else {
        /* UTF-8 string. */

        if (index < 0 || index == length) {
            index = (search_length == 0) ? index : -1;
            goto done;
        }

        p = njs_string_offset(string.start, end, index);

        for (; p >= string.start; p = njs_utf8_prev(p)) {
            if ((p + s.size) <= end && memcmp(p, s.start, s.size) == 0) {
                goto done;
            }

            index--;
        }

        index = -1;
    }

done:

    njs_set_number(&vm->retval, index);

    return NJS_OK;
}