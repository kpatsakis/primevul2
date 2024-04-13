njs_string_slice_string_prop(njs_string_prop_t *dst,
    const njs_string_prop_t *string, const njs_slice_prop_t *slice)
{
    size_t        size, n, length;
    const u_char  *p, *start, *end;

    length = slice->length;
    start = string->start;

    if (string->size == slice->string_length) {
        /* Byte or ASCII string. */
        start += slice->start;
        size = slice->length;

        if (string->length == 0) {
            /* Byte string. */
            length = 0;
        }

    } else {
        /* UTF-8 string. */
        end = start + string->size;

        if (slice->start < slice->string_length) {
            start = njs_string_offset(start, end, slice->start);

            /* Evaluate size of the slice in bytes and adjust length. */
            p = start;
            n = length;

            while (n != 0 && p < end) {
                p = njs_utf8_next(p, end);
                n--;
            }

            size = p - start;
            length -= n;

        } else {
            length = 0;
            size = 0;
        }
    }

    dst->start = (u_char *) start;
    dst->length = length;
    dst->size = size;
}