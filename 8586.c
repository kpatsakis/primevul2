njs_decode_utf8(njs_str_t *dst, const njs_str_t *src)
{
    njs_unicode_decode_t  ctx;

    njs_utf8_decode_init(&ctx);

    (void) njs_utf8_stream_encode(&ctx, src->start, src->start + src->length,
                                  dst->start, 1, 0);
}