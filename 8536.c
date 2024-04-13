njs_string_index_of(njs_string_prop_t *string, njs_string_prop_t *search,
    size_t from)
{
    size_t        index, length, search_length;
    const u_char  *p, *end;

    length = (string->length == 0) ? string->size : string->length;

    if (njs_slow_path(search->size == 0)) {
        return (from < length) ? from : length;
    }

    index = from;
    search_length = (search->length == 0) ? search->size : search->length;

    if (length - index >= search_length) {
        end = string->start + string->size;

        if (string->size == length) {
            /* Byte or ASCII string. */

            end -= (search->size - 1);

            for (p = string->start + index; p < end; p++) {
                if (memcmp(p, search->start, search->size) == 0) {
                    return index;
                }

                index++;
            }

        } else {
            /* UTF-8 string. */

            p = njs_string_offset(string->start, end, index);
            end -= search->size - 1;

            while (p < end) {
                if (memcmp(p, search->start, search->size) == 0) {
                    return index;
                }

                index++;
                p = njs_utf8_next(p, end);
            }
        }
    }

    return -1;
}