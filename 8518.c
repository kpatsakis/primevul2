njs_string_offset_map_init(const u_char *start, size_t size)
{
    size_t        offset;
    uint32_t      *map;
    njs_uint_t    n;
    const u_char  *p, *end;

    end = start + size;
    map = njs_string_map_start(end);
    p = start;
    n = 0;
    offset = NJS_STRING_MAP_STRIDE;

    do {
        if (offset == 0) {
            map[n++] = p - start;
            offset = NJS_STRING_MAP_STRIDE;
        }

        /* The UTF-8 string should be valid since its length is known. */
        p = njs_utf8_next(p, end);

        offset--;

    } while (p < end);
}