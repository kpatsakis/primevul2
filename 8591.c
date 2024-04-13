njs_string_base64url(njs_vm_t *vm, njs_value_t *value, const njs_str_t *src)
{
    size_t     padding;
    njs_str_t  dst;

    if (njs_slow_path(src->length == 0)) {
        vm->retval = njs_string_empty;
        return NJS_OK;
    }

    padding = src->length % 3;

    /*
     * Calculating the padding length: 0 -> 0, 1 -> 2, 2 -> 1.
     */
    padding = (4 >> padding) & 0x03;

    dst.length = njs_base64_encoded_length(src->length) - padding;

    dst.start = njs_string_alloc(vm, value, dst.length, dst.length);
    if (njs_slow_path(dst.start == NULL)) {
        return NJS_ERROR;
    }

    njs_encode_base64url(&dst, src);

    return NJS_OK;
}