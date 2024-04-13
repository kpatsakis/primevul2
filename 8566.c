njs_encode_hex_length(const njs_str_t *src, size_t *out_size)
{
    size_t  size;

    size = src->length * 2;

    if (out_size != NULL) {
        *out_size = size;
    }

    return size;
}