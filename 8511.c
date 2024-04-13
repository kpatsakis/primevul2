njs_string_eq(const njs_value_t *v1, const njs_value_t *v2)
{
    size_t        size, length1, length2;
    const u_char  *start1, *start2;

    size = v1->short_string.size;

    if (size != v2->short_string.size) {
        return 0;
    }

    if (size != NJS_STRING_LONG) {
        length1 = v1->short_string.length;
        length2 = v2->short_string.length;

        /*
         * Using full memcmp() comparison if at least one string
         * is a Byte string.
         */
        if (length1 != 0 && length2 != 0 && length1 != length2) {
            return 0;
        }

        start1 = v1->short_string.start;
        start2 = v2->short_string.start;

    } else {
        size = v1->long_string.size;

        if (size != v2->long_string.size) {
            return 0;
        }

        length1 = v1->long_string.data->length;
        length2 = v2->long_string.data->length;

        /*
         * Using full memcmp() comparison if at least one string
         * is a Byte string.
         */
        if (length1 != 0 && length2 != 0 && length1 != length2) {
            return 0;
        }

        start1 = v1->long_string.data->start;
        start2 = v2->long_string.data->start;
    }

    return (memcmp(start1, start2, size) == 0);
}