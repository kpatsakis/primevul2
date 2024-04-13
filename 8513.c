njs_string_prototype_char_at(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    size_t             length;
    int64_t            start;
    njs_int_t          ret;
    njs_slice_prop_t   slice;
    njs_string_prop_t  string;

    ret = njs_string_object_validate(vm, njs_argument(args, 0));
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    slice.string_length = njs_string_prop(&string, njs_argument(args, 0));

    ret = njs_value_to_integer(vm, njs_arg(args, nargs, 1), &start);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    length = 1;

    if (start < 0 || start >= (int64_t) slice.string_length) {
        start = 0;
        length = 0;
    }

    slice.start = start;
    slice.length = length;

    return njs_string_slice(vm, &vm->retval, &string, &slice);
}