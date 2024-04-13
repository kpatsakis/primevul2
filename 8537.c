njs_string_decode_uri_cp(const int8_t *hex, const u_char **start,
    const u_char *end, njs_bool_t expect_percent)
{
    int8_t                d0, d1;
    uint32_t              cp;
    const u_char          *p;
    njs_unicode_decode_t  ctx;

    njs_utf8_decode_init(&ctx);

    cp = njs_utf8_decode(&ctx, start, end);
    if (njs_fast_path(cp != '%')) {
        return expect_percent ? NJS_UNICODE_ERROR : cp;
    }

    p = *start;

    if (njs_slow_path((p + 1) >= end)) {
        return NJS_UNICODE_ERROR;
    }

    d0 = hex[*p++];
    if (njs_slow_path(d0 < 0)) {
        return NJS_UNICODE_ERROR;
    }

    d1 = hex[*p++];
    if (njs_slow_path(d1 < 0)) {
        return NJS_UNICODE_ERROR;
    }

    *start += 2;

    return (d0 << 4) + d1;
}