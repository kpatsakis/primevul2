njs_string_decode_hex(njs_vm_t *vm, njs_value_t *value, const njs_str_t *src)
{
    size_t     size, length;
    njs_str_t  dst;

    length = njs_decode_hex_length(src, &size);

    if (njs_slow_path(size == 0)) {
        vm->retval = njs_string_empty;
        return NJS_OK;
    }

    dst.start = njs_string_alloc(vm, value, size, length);
    if (njs_slow_path(dst.start == NULL)) {
        return NJS_ERROR;
    }

    dst.length = size;

    njs_decode_hex(&dst, src);

    if (njs_slow_path(dst.length != size)) {
        njs_string_truncate(value, dst.length, 0);
    }

    return NJS_OK;
}