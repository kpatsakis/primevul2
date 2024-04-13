njs_string_decode_base64(njs_vm_t *vm, njs_value_t *value, const njs_str_t *src)
{
    return njs_string_decode_base64_core(vm, value, src, 0);
}