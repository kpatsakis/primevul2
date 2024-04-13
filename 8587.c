njs_string_prototype_trim(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t mode)
{
    uint32_t              u, trim, length;
    njs_int_t             ret;
    njs_value_t           *value;
    const u_char          *p, *prev, *start, *end;
    njs_string_prop_t     string;
    njs_unicode_decode_t  ctx;

    value = njs_argument(args, 0);
    ret = njs_string_object_validate(vm, value);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    trim = 0;

    njs_string_prop(&string, value);

    start = string.start;
    end = string.start + string.size;

    if (njs_is_byte_or_ascii_string(&string)) {

        if (mode & NJS_TRIM_START) {
            for ( ;; ) {
                if (start == end) {
                    goto empty;
                }

                if (njs_is_whitespace(*start)) {
                    start++;
                    trim++;
                    continue;
                }

                break;
            }
        }

        if (mode & NJS_TRIM_END) {
            for ( ;; ) {
                if (start == end) {
                    goto empty;
                }

                end--;

                if (njs_is_whitespace(*end)) {
                    trim++;
                    continue;
                }

                end++;
                break;
            }
        }

    } else {
        /* UTF-8 string. */

        if (mode & NJS_TRIM_START) {
            njs_utf8_decode_init(&ctx);

            for ( ;; ) {
                if (start == end) {
                    goto empty;
                }

                p = start;
                u = njs_utf8_decode(&ctx, &start, end);

                if (njs_utf8_is_whitespace(u)) {
                    trim++;
                    continue;
                }

                start = p;
                break;
            }
        }

        if (mode & NJS_TRIM_END) {
            prev = end;

            njs_utf8_decode_init(&ctx);

            for ( ;; ) {
                if (start == prev) {
                    goto empty;
                }

                prev = njs_utf8_prev(prev);
                p = prev;
                u = njs_utf8_decode(&ctx, &p, end);

                if (njs_utf8_is_whitespace(u)) {
                    trim++;
                    continue;
                }

                end = p;
                break;
            }
        }
    }

    if (trim == 0) {
        /* GC: retain. */
        vm->retval = *value;

        return NJS_OK;
    }

    length = (string.length != 0) ? string.length - trim : 0;

    return njs_string_new(vm, &vm->retval, start, end - start, length);

empty:

    vm->retval = njs_string_empty;

    return NJS_OK;
}