njs_decode_utf8_length(const njs_str_t *src, size_t *out_size)
{
    njs_unicode_decode_t  ctx;

    njs_utf8_decode_init(&ctx);

    return njs_utf8_stream_length(&ctx, src->start, src->length, 1, 0,
                                  out_size);
}