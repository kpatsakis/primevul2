njs_decode_base64url_length(const njs_str_t *src, size_t *out_size)
{
    return njs_decode_base64_length_core(src, njs_basis64url, out_size);
}