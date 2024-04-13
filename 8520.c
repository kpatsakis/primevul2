njs_encode_base64_core(njs_str_t *dst, const njs_str_t *src,
    const u_char *basis, njs_bool_t padding)
{
   u_char  *d, *s, c0, c1, c2;
   size_t  len;

    len = src->length;
    s = src->start;
    d = dst->start;

    while (len > 2) {
        c0 = s[0];
        c1 = s[1];
        c2 = s[2];

        *d++ = basis[c0 >> 2];
        *d++ = basis[((c0 & 0x03) << 4) | (c1 >> 4)];
        *d++ = basis[((c1 & 0x0f) << 2) | (c2 >> 6)];
        *d++ = basis[c2 & 0x3f];

        s += 3;
        len -= 3;
    }

    if (len > 0) {
        c0 = s[0];
        *d++ = basis[c0 >> 2];

        if (len == 1) {
            *d++ = basis[(c0 & 0x03) << 4];
            if (padding) {
                *d++ = '=';
                *d++ = '=';
            }

        } else {
            c1 = s[1];

            *d++ = basis[((c0 & 0x03) << 4) | (c1 >> 4)];
            *d++ = basis[(c1 & 0x0f) << 2];

            if (padding) {
                *d++ = '=';
            }
        }

    }

    dst->length = d - dst->start;
}