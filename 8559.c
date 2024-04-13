njs_decode_base64_core(njs_str_t *dst, const njs_str_t *src,
    const u_char *basis)
{
    size_t  len;
    u_char  *d, *s;

    s = src->start;
    d = dst->start;

    len = dst->length;

    while (len >= 3) {
        *d++ = (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
        *d++ = (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
        *d++ = (u_char) (basis[s[2]] << 6 | basis[s[3]]);

        s += 4;
        len -= 3;
    }

    if (len >= 1) {
        *d++ = (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
    }

    if (len >= 2) {
        *d++ = (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
    }
}