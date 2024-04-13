njs_string_base64(njs_vm_t *vm, njs_value_t *value, const njs_str_t *src)
{
    size_t     length;
    njs_str_t  dst;

    length = njs_encode_base64_length(src, &dst.length);

    if (njs_slow_path(dst.length == 0)) {
        vm->retval = njs_string_empty;
        return NJS_OK;
    }

    dst.start = njs_string_alloc(vm, value, dst.length, length);
    if (njs_slow_path(dst.start == NULL)) {
        return NJS_ERROR;
    }

    njs_encode_base64(&dst, src);

    return NJS_OK;
}