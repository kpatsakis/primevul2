njs_string_hex(njs_vm_t *vm, njs_value_t *value, const njs_str_t *src)
{
    size_t     length;
    njs_str_t  dst;

    length = njs_encode_hex_length(src, &dst.length);

    dst.start = njs_string_alloc(vm, value, dst.length, length);
    if (njs_fast_path(dst.start != NULL)) {
        njs_encode_hex(&dst, src);
        return NJS_OK;
    }

    return NJS_ERROR;
}