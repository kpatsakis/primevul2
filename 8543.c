njs_string_match_multiple(njs_vm_t *vm, njs_value_t *args,
    njs_regexp_pattern_t *pattern)
{
    size_t             c0, c1;
    int32_t            size, length;
    njs_int_t          ret;
    njs_utf8_t         utf8;
    njs_array_t        *array;
    const u_char       *p, *start, *end;
    njs_regexp_utf8_t  type;
    njs_string_prop_t  string;

    njs_set_number(&args[1].data.u.regexp->last_index, 0);
    vm->retval = njs_value_null;

    (void) njs_string_prop(&string, &args[0]);

    utf8 = NJS_STRING_BYTE;
    type = NJS_REGEXP_BYTE;

    if (string.length != 0) {
        utf8 = NJS_STRING_ASCII;
        type = NJS_REGEXP_UTF8;

        if (string.length != string.size) {
            utf8 = NJS_STRING_UTF8;
        }
    }

    if (njs_regex_is_valid(&pattern->regex[type])) {

        array = njs_array_alloc(vm, 0, 0, NJS_ARRAY_SPARE);
        if (njs_slow_path(array == NULL)) {
            return NJS_ERROR;
        }

        p = string.start;
        end = p + string.size;

        do {
            ret = njs_regexp_match(vm, &pattern->regex[type], p, 0, string.size,
                                   vm->single_match_data);
            if (ret < 0) {
                if (njs_fast_path(ret == NJS_DECLINED)) {
                    break;
                }

                njs_internal_error(vm, "njs_regexp_match() failed");

                return NJS_ERROR;
            }

            ret = njs_array_expand(vm, array, 0, 1);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }

            c0 = njs_regex_capture(vm->single_match_data, 0);
            c1 = njs_regex_capture(vm->single_match_data, 1);
            start = p + c0;

            if (c1 == 0) {
                if (start < end) {
                    p = (utf8 != NJS_STRING_BYTE) ? njs_utf8_next(start, end)
                                                  : start + 1;
                    string.size = end - p;

                } else {
                    /* To exit the loop. */
                    p++;
                }

                size = 0;
                length = 0;

            } else {
                p += c1;
                string.size -= c1;

                size = c1 - c0;
                length = njs_string_calc_length(utf8, start, size);
            }

            ret = njs_string_new(vm, &array->start[array->length],
                                 start, size, length);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }

            array->length++;

        } while (p <= end);

        njs_set_array(&vm->retval, array);
    }

    return NJS_OK;
}