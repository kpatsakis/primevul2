njs_string_alloc(njs_vm_t *vm, njs_value_t *value, uint64_t size,
    uint64_t length)
{
    uint32_t      total, map_offset, *map;
    njs_string_t  *string;

    if (njs_slow_path(size > NJS_STRING_MAX_LENGTH)) {
        njs_range_error(vm, "invalid string length");
        return NULL;
    }

    value->type = NJS_STRING;
    njs_string_truth(value, size);

    if (size <= NJS_STRING_SHORT) {
        value->short_string.size = size;
        value->short_string.length = length;

        return value->short_string.start;
    }

    /*
     * Setting UTF-8 length is not required here, it just allows
     * to store the constant in whole byte instead of bit twiddling.
     */
    value->short_string.size = NJS_STRING_LONG;
    value->short_string.length = 0;
    value->long_string.external = 0;
    value->long_string.size = size;

    if (size != length && length > NJS_STRING_MAP_STRIDE) {
        map_offset = njs_string_map_offset(size);
        total = map_offset + njs_string_map_size(length);

    } else {
        map_offset = 0;
        total = size;
    }

    string = njs_mp_alloc(vm->mem_pool, sizeof(njs_string_t) + total);

    if (njs_fast_path(string != NULL)) {
        value->long_string.data = string;

        string->start = (u_char *) string + sizeof(njs_string_t);
        string->length = length;
        string->retain = 1;

        if (map_offset != 0) {
            map = (uint32_t *) (string->start + map_offset);
            map[0] = 0;
        }

        return string->start;
    }

    njs_memory_error(vm);

    return NULL;
}