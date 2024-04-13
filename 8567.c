njs_decode_hex_length(const njs_str_t *src, size_t *out_size)
{
    if (out_size != NULL) {
        *out_size = src->length / 2;
    }

    return 0;
}