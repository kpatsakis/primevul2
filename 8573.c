njs_encode_base64_length(const njs_str_t *src, size_t *out_size)
{
    size_t  size;

    size = (src->length == 0) ? 0 : njs_base64_encoded_length(src->length);

    if (out_size != NULL) {
        *out_size = size;
    }

    return size;
}