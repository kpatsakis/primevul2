njs_string_create(njs_vm_t *vm, njs_value_t *value, const char *src,
    size_t size)
{
    njs_str_t  str;

    str.start = (u_char *) src;
    str.length = size;

    return njs_string_decode_utf8(vm, value, &str);
}