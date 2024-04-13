njs_string_decode_base64_core(njs_vm_t *vm, njs_value_t *value,
    const njs_str_t *src, njs_bool_t url)
{
    size_t     length;
    const u_char *basis;
    njs_str_t  dst;

    basis = (url) ? njs_basis64url : njs_basis64;

    length = njs_decode_base64_length_core(src, basis, &dst.length);

    if (njs_slow_path(dst.length == 0)) {
        vm->retval = njs_string_empty;
        return NJS_OK;
    }

    dst.start = njs_string_alloc(vm, value, dst.length, length);
    if (njs_slow_path(dst.start == NULL)) {
        return NJS_ERROR;
    }

    njs_decode_base64_core(&dst, src, basis);

    return NJS_OK;
}