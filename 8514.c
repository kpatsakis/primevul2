njs_string_encode_uri(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t component)
{
    u_char                byte, *dst;
    uint64_t              size;
    uint32_t              cp, cp_low;
    njs_int_t             ret;
    njs_value_t           *value;
    const u_char          *src, *end;
    const uint32_t        *escape;
    njs_string_prop_t     string;
    njs_unicode_decode_t  ctx;
    u_char                encode[4];

    static const uint32_t  escape_uri[] = {
        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                     /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0x50000025,  /* 0101 0000 0000 0000  0000 0000 0010 0101 */

                     /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x78000000,  /* 0111 1000 0000 0000  0000 0000 0000 0000 */

                     /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0xb8000001,  /* 1011 1000 0000 0000  0000 0000 0000 0001 */

        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    };

    static const uint32_t  escape_uri_component[] = {
        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */

                     /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
        0xfc00987d,  /* 1111 1100 0000 0000  1001 1000 0111 1101 */

                     /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
        0x78000001,  /* 0111 1000 0000 0000  0000 0000 0000 0001 */

                     /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
        0xb8000001,  /* 1011 1000 0000 0000  0000 0000 0000 0001 */

        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
        0xffffffff,  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
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

    escape = (component) ? escape_uri_component : escape_uri;

    njs_prefetch(escape);

    (void) njs_string_prop(&string, value);

    size = 0;
    src = string.start;
    end = src + string.size;

    if (njs_is_byte_or_ascii_string(&string)) {

        while (src < end) {
            byte = *src++;
            size += njs_need_escape(escape, byte) ? 3 : 1;
        }

    } else {
        /* UTF-8 string. */

        njs_utf8_decode_init(&ctx);

        while (src < end) {
            cp = njs_utf8_decode(&ctx, &src, end);

            if (cp < 0x80 && !njs_need_escape(escape, cp)) {
                size++;
                continue;
            }

            if (njs_slow_path(njs_surrogate_any(cp))) {
                if (src == end) {
                    goto uri_error;
                }

                if (njs_surrogate_leading(cp)) {
                    cp_low = njs_utf8_decode(&ctx, &src, end);

                    if (njs_slow_path(!njs_surrogate_trailing(cp_low))) {
                        goto uri_error;
                    }

                    cp = njs_surrogate_pair(cp, cp_low);
                    size += njs_utf8_size(cp) * 3;
                    continue;
                }

                goto uri_error;
            }

            size += njs_utf8_size(cp) * 3;
        }
    }

    if (size == 0) {
        /* GC: retain src. */
        vm->retval = *value;
        return NJS_OK;
    }

    dst = njs_string_alloc(vm, &vm->retval, size, size);
    if (njs_slow_path(dst == NULL)) {
        return NJS_ERROR;
    }

    src = string.start;

    if (njs_is_byte_or_ascii_string(&string)) {
        (void) njs_string_encode(escape, string.size, src, dst);
        return NJS_OK;
    }

    /* UTF-8 string. */

    njs_utf8_decode_init(&ctx);

    while (src < end) {
        cp = njs_utf8_decode(&ctx, &src, end);

        if (njs_slow_path(njs_surrogate_leading(cp))) {
            cp_low = njs_utf8_decode(&ctx, &src, end);
            cp = njs_surrogate_pair(cp, cp_low);
        }

        njs_utf8_encode(encode, cp);

        dst = njs_string_encode(escape, njs_utf8_size(cp), encode, dst);
    }

    return NJS_OK;

uri_error:

    njs_uri_error(vm, "malformed URI");

    return NJS_ERROR;
}