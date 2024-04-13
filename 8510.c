njs_string_prototype_repeat(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    u_char             *p;
    int64_t            n, max;
    uint64_t           size, length;
    njs_int_t          ret;
    njs_value_t        *this;
    njs_string_prop_t  string;

    this = njs_argument(args, 0);

    if (njs_slow_path(njs_is_null_or_undefined(this))) {
        njs_type_error(vm, "cannot convert \"%s\"to object",
                       njs_type_string(this->type));
        return NJS_ERROR;
    }

    ret = njs_value_to_string(vm, this, this);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    ret = njs_value_to_integer(vm, njs_arg(args, nargs, 1), &n);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    if (njs_slow_path(n < 0 || n == INT64_MAX)) {
        njs_range_error(vm, NULL);
        return NJS_ERROR;
    }

    (void) njs_string_prop(&string, this);

    if (njs_slow_path(n == 0 || string.size == 0)) {
        vm->retval = njs_string_empty;
        return NJS_OK;
    }

    max = NJS_STRING_MAX_LENGTH / string.size;

    if (njs_slow_path(n >= max)) {
        njs_range_error(vm, NULL);
        return NJS_ERROR;
    }

    size = string.size * n;
    length = string.length * n;

    p = njs_string_alloc(vm, &vm->retval, size, length);
    if (njs_slow_path(p == NULL)) {
        return NJS_ERROR;
    }

    while (n != 0) {
        p = memcpy(p, string.start, string.size);
        p += string.size;
        n--;
    }

    return NJS_OK;
}