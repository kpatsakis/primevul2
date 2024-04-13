njs_string_get_substitution(njs_vm_t *vm, njs_value_t *matched,
    njs_value_t *string, int64_t pos, njs_value_t *captures, int64_t ncaptures,
    njs_value_t *groups, njs_value_t *replacement, njs_value_t *retval)
{
    int64_t      tail, size, length, n;
    u_char       c, c2, *p, *r, *end;
    njs_str_t    rep, m, str, cap;
    njs_int_t    ret;
    njs_chb_t    chain;
    njs_value_t  name, value;

    njs_string_get(replacement, &rep);
    p = rep.start;
    end = rep.start + rep.length;

    njs_chb_init(&chain, vm->mem_pool);

    while (p < end) {
        r = njs_strlchr(p, end, '$');
        if (r == NULL || r == &end[-1]) {
            if (njs_fast_path(p == rep.start)) {
                *retval = *replacement;
                return NJS_OK;
            }

            njs_chb_append(&chain, p, end - p);
            goto done;
        }

        njs_chb_append(&chain, p, r - p);
        p = r;

        c = r[1];

        switch (c) {
        case '$':
            njs_chb_append_literal(&chain, "$");
            p += 2;
            break;

        case '&':
            njs_string_get(matched, &m);
            njs_chb_append_str(&chain, &m);
            p += 2;
            break;

        case '`':
            njs_string_get(string, &str);
            njs_chb_append(&chain, str.start, pos);
            p += 2;
            break;

        case '\'':
            njs_string_get(matched, &m);
            tail = pos + m.length;

            njs_string_get(string, &str);
            njs_chb_append(&chain, &str.start[tail],
                           njs_max((int64_t) str.length - tail, 0));
            p += 2;
            break;

        case '<':
            r = njs_strlchr(p, end, '>');
            if (groups == NULL || njs_is_undefined(groups) || r == NULL) {
                njs_chb_append(&chain, p, 2);
                p += 2;
                break;
            }

            p += 2;

            ret = njs_vm_value_string_set(vm, &name, p, r - p);
            if (njs_slow_path(ret != NJS_OK)) {
                goto exception;
            }

            p = r + 1;

            ret = njs_value_property(vm, groups, &name, &value);
            if (njs_slow_path(ret == NJS_ERROR)) {
                goto exception;
            }

            if (njs_is_defined(&value)) {
                ret = njs_value_to_string(vm, &value, &value);
                if (njs_slow_path(ret == NJS_ERROR)) {
                    goto exception;
                }

                njs_string_get(&value, &str);
                njs_chb_append_str(&chain, &str);
            }

            break;

        default:
            if (c >= '0' && c <= '9') {
                n = c - '0';

                c2 = (&r[2] < end) ? r[2] : 0;

                if (c2 >= '0' && c2 <= '9'
                    && (n * 10 + (c2 - '0')) <= ncaptures)
                {
                    n = n * 10 + (c2 - '0');

                } else {
                    c2 = 0;
                }

                if (n == 0 || n > ncaptures) {
                    njs_chb_append(&chain, p, (c2 != 0) ? 3 : 2);
                    p += (c2 != 0) ? 3 : 2;
                    break;
                }

                p += (c2 != 0) ? 3 : 2;

                if (njs_is_defined(&captures[n])) {
                    njs_string_get(&captures[n], &cap);
                    njs_chb_append_str(&chain, &cap);
                }

                break;
            }

            njs_chb_append_literal(&chain, "$");
            p += 1;
            break;
        }
    }

done:

    size = njs_chb_size(&chain);
    if (njs_slow_path(size < 0)) {
        njs_memory_error(vm);
        ret = NJS_ERROR;
        goto exception;
    }

    length = njs_chb_utf8_length(&chain);

    p = njs_string_alloc(vm, retval, size, length);
    if (njs_slow_path(p == NULL)) {
        ret = NJS_ERROR;
        goto exception;
    }

    njs_chb_join_to(&chain, p);

    ret = NJS_OK;

exception:

    njs_chb_destroy(&chain);

    return ret;
}