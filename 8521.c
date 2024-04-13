njs_string_set(njs_vm_t *vm, njs_value_t *value, const u_char *start,
    uint32_t size)
{
    u_char        *dst;
    const u_char  *src;
    njs_string_t  *string;

    value->type = NJS_STRING;
    njs_string_truth(value, size);

    if (size <= NJS_STRING_SHORT) {
        value->short_string.size = size;
        value->short_string.length = 0;

        dst = value->short_string.start;
        src = start;

        while (size != 0) {
            /* The maximum size is just 14 bytes. */
            njs_pragma_loop_disable_vectorization;

            *dst++ = *src++;
            size--;
        }

    } else {
        /*
         * Setting UTF-8 length is not required here, it just allows
         * to store the constant in whole byte instead of bit twiddling.
         */
        value->short_string.size = NJS_STRING_LONG;
        value->short_string.length = 0;
        value->long_string.external = 0xff;
        value->long_string.size = size;

        string = njs_mp_alloc(vm->mem_pool, sizeof(njs_string_t));
        if (njs_slow_path(string == NULL)) {
            njs_memory_error(vm);
            return NJS_ERROR;
        }

        value->long_string.data = string;

        string->start = (u_char *) start;
        string->length = 0;
        string->retain = 1;
    }

    return NJS_OK;
}