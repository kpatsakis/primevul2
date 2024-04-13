njs_decode_base64(njs_str_t *dst, const njs_str_t *src)
{
    njs_decode_base64_core(dst, src, njs_basis64);
}