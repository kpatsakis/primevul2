njs_string_truncate(njs_value_t *value, uint32_t size, uint32_t length)
{
    u_char    *dst, *src;
    uint32_t  n;

    if (size <= NJS_STRING_SHORT) {
        if (value->short_string.size == NJS_STRING_LONG) {
            dst = value->short_string.start;
            src = value->long_string.data->start;

            n = size;

            while (n != 0) {
                /* The maximum size is just 14 bytes. */
                njs_pragma_loop_disable_vectorization;

                *dst++ = *src++;
                n--;
            }
        }

        value->short_string.size = size;
        value->short_string.length = length;

    } else {
        value->long_string.size = size;
        value->long_string.data->length = length;
    }
}