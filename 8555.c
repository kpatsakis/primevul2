njs_string_slice_prop(njs_vm_t *vm, njs_string_prop_t *string,
    njs_slice_prop_t *slice, njs_value_t *args, njs_uint_t nargs)
{
    slice->string_length = njs_string_prop(string, &args[0]);

    return njs_string_slice_args(vm, slice, args, nargs);
}