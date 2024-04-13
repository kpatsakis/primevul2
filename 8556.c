njs_encode_base64url(njs_str_t *dst, const njs_str_t *src)
{
    static u_char   basis64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    njs_encode_base64_core(dst, src, basis64, 0);
}