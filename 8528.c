njs_string_prototype_index_of(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    int64_t            from, length;
    njs_int_t          ret;
    njs_value_t        *this, *search, *pos, search_lvalue, pos_lvalue;
    njs_string_prop_t  string, s;

    this = njs_argument(args, 0);

    if (njs_slow_path(njs_is_null_or_undefined(this))) {
        njs_type_error(vm, "cannot convert \"%s\"to object",
                       njs_type_string(this->type));
        return NJS_ERROR;
    }

    ret = njs_value_to_string(vm, this, this);
    if (njs_slow_path(ret != NJS_OK)) {
        return NJS_ERROR;
    }

    search = njs_lvalue_arg(&search_lvalue, args, nargs, 1);
    ret = njs_value_to_string(vm, search, search);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    pos = njs_lvalue_arg(&pos_lvalue, args, nargs, 2);
    ret = njs_value_to_integer(vm, pos, &from);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    length = njs_string_prop(&string, this);
    (void) njs_string_prop(&s, search);

    from = njs_min(njs_max(from, 0), length);

    njs_set_number(&vm->retval, njs_string_index_of(&string, &s, from));

    return NJS_OK;
}