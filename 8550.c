njs_string_new(njs_vm_t *vm, njs_value_t *value, const u_char *start,
    uint32_t size, uint32_t length)
{
    u_char  *p;

    p = njs_string_alloc(vm, value, size, length);

    if (njs_fast_path(p != NULL)) {
        memcpy(p, start, size);
        return NJS_OK;
    }

    return NJS_ERROR;
}