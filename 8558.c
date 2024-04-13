njs_string_offset(const u_char *start, const u_char *end, size_t index)
{
    uint32_t    *map;
    njs_uint_t  skip;

    if (index >= NJS_STRING_MAP_STRIDE) {
        map = njs_string_map_start(end);

        if (map[0] == 0) {
            njs_string_offset_map_init(start, end - start);
        }

        start += map[index / NJS_STRING_MAP_STRIDE - 1];
    }

    for (skip = index % NJS_STRING_MAP_STRIDE; skip != 0; skip--) {
        start = njs_utf8_next(start, end);
    }

    return start;
}