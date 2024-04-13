njs_decode_base64_length_core(const njs_str_t *src, const u_char *basis,
    size_t *out_size)
{
    uint    pad;
    size_t  len;

    for (len = 0; len < src->length; len++) {
        if (basis[src->start[len]] == 77) {
            break;
        }
    }

    pad = 0;

    if (len % 4 != 0) {
        pad = 4 - (len % 4);
        len += pad;
    }

    len = njs_base64_decoded_length(len, pad);

    if (out_size != NULL) {
        *out_size = len;
    }

    return 0;
}