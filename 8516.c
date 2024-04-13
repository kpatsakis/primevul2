njs_string_decode_uri(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t component)
{
    u_char                *dst;
    int64_t               size, length;
    uint32_t              cp;
    njs_int_t             ret;
    njs_chb_t             chain;
    njs_uint_t            i, n;
    njs_bool_t            percent;
    njs_value_t           *value;
    const u_char          *src, *p, *end;
    const uint32_t        *reserve;
    njs_string_prop_t     string;
    njs_unicode_decode_t  ctx;
    u_char                encode[4];

    static const uint32_t  reserve_uri[] = {
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                     /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0xac009858,  /* 1010 1100 0000 0000  1001 1000 0101 1000 */

                     /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000001,  /* 0000 0000 0000 0000  0000 0000 0000 0001 */

                     /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */
    };

    static const uint32_t  reserve_uri_component[] = {
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                     /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                     /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */

                     /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */

        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */
        0x00000000,  /* 0000 0000 0000 0000  0000 0000 0000 0000 */
    };

    static const int8_t  hex[256]
        njs_aligned(32) =
    {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };

    if (nargs < 2) {
        vm->retval = njs_string_undefined;
        return NJS_OK;
    }

    value = njs_argument(args, 1);
    ret = njs_value_to_string(vm, value, value);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    reserve = component ? reserve_uri_component : reserve_uri;

    njs_prefetch(reserve);
    njs_prefetch(&hex['0']);

    (void) njs_string_prop(&string, value);

    length = 0;
    src = string.start;
    end = string.start + string.size;

    njs_chb_init(&chain, vm->mem_pool);

    njs_utf8_decode_init(&ctx);

    while (src < end) {
        percent = (src[0] == '%');
        cp = njs_string_decode_uri_cp(hex, &src, end, 0);
        if (njs_slow_path(cp > NJS_UNICODE_MAX_CODEPOINT)) {
            goto uri_error;
        }

        if (!percent) {
            length += 1;
            dst = njs_chb_reserve(&chain, 4);
            if (dst != NULL) {
                njs_utf8_encode(dst, cp);
                njs_chb_written(&chain, njs_utf8_size(cp));
            }

            continue;
        }

        if (cp < 0x80) {
            if (njs_reserved(reserve, cp)) {
                length += 3;
                njs_chb_append(&chain, &src[-3], 3);

            } else {
                length += 1;
                dst = njs_chb_reserve(&chain, 1);
                if (dst != NULL) {
                    *dst = cp;
                    njs_chb_written(&chain, 1);
                }
            }

            continue;
        }

        n = 1;

        do {
            n++;
        } while (((cp << n) & 0x80));

        if (njs_slow_path(n > 4)) {
            goto uri_error;
        }

        encode[0] = cp;

        for (i = 1; i < n; i++) {
            cp = njs_string_decode_uri_cp(hex, &src, end, 1);
            if (njs_slow_path(cp > NJS_UNICODE_MAX_CODEPOINT)) {
                goto uri_error;
            }

            encode[i] = cp;
        }

        p = encode;
        cp = njs_utf8_decode(&ctx, &p, p + n);
        if (njs_slow_path(cp > NJS_UNICODE_MAX_CODEPOINT)) {
            goto uri_error;
        }

        dst = njs_chb_reserve(&chain, 4);
        if (dst != NULL) {
            njs_utf8_encode(dst, cp);
            njs_chb_written(&chain, njs_utf8_size(cp));
        }

        length += 1;
    }

    size = njs_chb_size(&chain);
    if (njs_slow_path(size < 0)) {
        njs_memory_error(vm);
        return NJS_ERROR;
    }

    if (size == 0) {
        /* GC: retain src. */
        vm->retval = *value;
        return NJS_OK;
    }

    dst = njs_string_alloc(vm, &vm->retval, size, length);
    if (njs_slow_path(dst == NULL)) {
        return NJS_ERROR;
    }

    njs_chb_join_to(&chain, dst);
    njs_chb_destroy(&chain);

    return NJS_OK;

uri_error:

    njs_uri_error(vm, "malformed URI");

    return NJS_ERROR;
}