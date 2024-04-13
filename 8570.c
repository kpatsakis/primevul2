njs_encode_hex(njs_str_t *dst, const njs_str_t *src)
{
    u_char        *p, c;
    size_t        i, len;
    const u_char  *start;

    static const u_char  hex[16] = "0123456789abcdef";

    len = src->length;
    start = src->start;

    p = dst->start;

    for (i = 0; i < len; i++) {
        c = start[i];
        *p++ = hex[c >> 4];
        *p++ = hex[c & 0x0f];
    }
}