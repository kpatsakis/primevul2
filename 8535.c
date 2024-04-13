njs_string_slice(njs_vm_t *vm, njs_value_t *dst,
    const njs_string_prop_t *string, const njs_slice_prop_t *slice)
{
    njs_string_prop_t  prop;

    njs_string_slice_string_prop(&prop, string, slice);

    if (njs_fast_path(prop.size != 0)) {
        return njs_string_new(vm, dst, prop.start, prop.size, prop.length);
    }

    *dst = njs_string_empty;

    return NJS_OK;
}