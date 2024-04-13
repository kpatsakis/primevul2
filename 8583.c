njs_decode_hex(njs_str_t *dst, const njs_str_t *src)
{
    u_char        *p;
    size_t        len;
    njs_int_t     c;
    njs_uint_t    i, n;
    const u_char  *start;

    n = 0;
    p = dst->start;

    start = src->start;
    len = src->length;

    for (i = 0; i < len; i++) {
        c = njs_char_to_hex(start[i]);
        if (njs_slow_path(c < 0)) {
            break;
        }

        n = n * 16 + c;

        if ((i & 1) != 0) {
            *p++ = (u_char) n;
            n = 0;
        }
    }

    dst->length -= (dst->start + dst->length) - p;
}