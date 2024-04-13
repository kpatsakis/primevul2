njs_string_validate(njs_vm_t *vm, njs_string_prop_t *string, njs_value_t *value)
{
    u_char    *start;
    size_t    new_size, map_offset;
    ssize_t   size, length;
    uint32_t  *map;

    size = value->short_string.size;

    if (size != NJS_STRING_LONG) {
        string->start = value->short_string.start;
        length = value->short_string.length;

        if (length == 0 && length != size) {
            length = njs_utf8_length(value->short_string.start, size);

            if (njs_slow_path(length < 0)) {
                /* Invalid UTF-8 string. */
                return length;
            }

            value->short_string.length = length;
        }

    } else {
        string->start = value->long_string.data->start;
        size = value->long_string.size;
        length = value->long_string.data->length;

        if (length == 0 && length != size) {
            length = njs_utf8_length(string->start, size);

            if (length != size) {
                if (njs_slow_path(length < 0)) {
                    /* Invalid UTF-8 string. */
                    return length;
                }

                if (length > NJS_STRING_MAP_STRIDE) {
                    /*
                     * Reallocate the long string with offset map
                     * after the string.
                     */
                    map_offset = njs_string_map_offset(size);
                    new_size = map_offset + njs_string_map_size(length);

                    start = njs_mp_alloc(vm->mem_pool, new_size);
                    if (njs_slow_path(start == NULL)) {
                        njs_memory_error(vm);
                        return NJS_ERROR;
                    }

                    memcpy(start, string->start, size);
                    string->start = start;
                    value->long_string.data->start = start;

                    map = (uint32_t *) (start + map_offset);
                    map[0] = 0;
                }
            }

            value->long_string.data->length = length;
        }
    }

    string->size = size;
    string->length = length;

    return length;
}