njs_string_to_c_string(njs_vm_t *vm, njs_value_t *value)
{
    u_char  *p, *data, *start;
    size_t  size;

    if (value->short_string.size != NJS_STRING_LONG) {
        start = value->short_string.start;
        size = value->short_string.size;

        if (size < NJS_STRING_SHORT) {
            start[size] = '\0';
            return (const char *) start;
        }

    } else {
        start = value->long_string.data->start;
        size = value->long_string.size;
    }

    data = njs_mp_alloc(vm->mem_pool, size + 1);
    if (njs_slow_path(data == NULL)) {
        njs_memory_error(vm);
        return NULL;
    }

    p = njs_cpymem(data, start, size);
    *p++ = '\0';

    return (const char *) data;
}