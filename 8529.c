njs_string_from_char_code(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t is_point)
{
    double                num;
    u_char                *p, *start, *end;
    ssize_t               len;
    int32_t               code;
    uint32_t              cp;
    uint64_t              length, size;
    njs_int_t             ret;
    njs_uint_t            i;
    njs_unicode_decode_t  ctx;
    u_char                buf[4];

    size = 0;
    length = 0;

    cp = 0x00;
    end = buf + sizeof(buf);

    njs_utf16_decode_init(&ctx);

    for (i = 1; i < nargs; i++) {
        if (!njs_is_numeric(&args[i])) {
            ret = njs_value_to_numeric(vm, &args[i], &args[i]);
            if (ret != NJS_OK) {
                return ret;
            }
        }

        if (is_point) {
            num = njs_number(&args[i]);
            if (isnan(num)) {
                goto range_error;
            }

            code = num;

            if (code != num || code < 0 || code > 0x10FFFF) {
                goto range_error;
            }

        } else {
            code = njs_number_to_uint16(njs_number(&args[i]));
        }

        start = buf;
        len = njs_utf16_encode(code, &start, end);

        start = buf;
        cp = njs_utf16_decode(&ctx, (const u_char **) &start, start + len);

        if (cp > NJS_UNICODE_MAX_CODEPOINT) {
            if (cp == NJS_UNICODE_CONTINUE) {
                continue;
            }

            cp = NJS_UNICODE_REPLACEMENT;
        }

        size += njs_utf8_size(cp);
        length++;
    }

    if (cp == NJS_UNICODE_CONTINUE) {
        size += njs_utf8_size(NJS_UNICODE_REPLACEMENT);
        length++;
    }

    p = njs_string_alloc(vm, &vm->retval, size, length);
    if (njs_slow_path(p == NULL)) {
        return NJS_ERROR;
    }

    njs_utf16_decode_init(&ctx);

    for (i = 1; i < nargs; i++) {
        if (is_point) {
            code = njs_number(&args[i]);

        } else {
            code = njs_number_to_uint16(njs_number(&args[i]));
        }

        start = buf;
        len = njs_utf16_encode(code, &start, end);

        start = buf;
        cp = njs_utf16_decode(&ctx, (const u_char **) &start, start + len);

        if (cp > NJS_UNICODE_MAX_CODEPOINT) {
            if (cp == NJS_UNICODE_CONTINUE && i + 1 != nargs) {
                continue;
            }

            cp = NJS_UNICODE_REPLACEMENT;
        }

        p = njs_utf8_encode(p, cp);
    }

    return NJS_OK;

range_error:

    njs_range_error(vm, NULL);

    return NJS_ERROR;
}