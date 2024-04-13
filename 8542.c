njs_string_index(njs_string_prop_t *string, uint32_t offset)
{
    uint32_t      *map, last, index;
    const u_char  *p, *start, *end;

    if (string->size == string->length) {
        return offset;
    }

    last = 0;
    index = 0;

    if (string->length > NJS_STRING_MAP_STRIDE) {

        end = string->start + string->size;
        map = njs_string_map_start(end);

        if (map[0] == 0) {
            njs_string_offset_map_init(string->start, string->size);
        }

        while (index + NJS_STRING_MAP_STRIDE < string->length
               && *map <= offset)
        {
            last = *map++;
            index += NJS_STRING_MAP_STRIDE;
        }
    }

    p = string->start + last;
    start = string->start + offset;
    end = string->start + string->size;

    while (p < start) {
        index++;
        p = njs_utf8_next(p, end);
    }

    return index;
}