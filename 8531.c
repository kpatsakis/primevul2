njs_string_prototype_to_string(njs_vm_t *vm, njs_value_t *args,
    njs_uint_t nargs, njs_index_t unused)
{
    njs_int_t          ret;
    njs_str_t          enc, str;
    njs_value_t        value;
    njs_string_prop_t  string;

    ret = njs_string_prototype_value_of(vm, args, nargs, unused);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    if (nargs < 2) {
        return NJS_OK;
    }

    if (njs_slow_path(!njs_is_string(&args[1]))) {
        njs_type_error(vm, "encoding must be a string");
        return NJS_ERROR;
    }

    value = vm->retval;

    (void) njs_string_prop(&string, &value);

    njs_string_get(&args[1], &enc);

    str.length = string.size;
    str.start = string.start;

    if (enc.length == 3 && memcmp(enc.start, "hex", 3) == 0) {
        return njs_string_hex(vm, &vm->retval, &str);

    } else if (enc.length == 6 && memcmp(enc.start, "base64", 6) == 0) {
        return njs_string_base64(vm, &vm->retval, &str);

    } else if (enc.length == 9 && memcmp(enc.start, "base64url", 9) == 0) {
        return njs_string_base64url(vm, &vm->retval, &str);
    }

    njs_type_error(vm, "Unknown encoding: \"%V\"", &enc);

    return NJS_ERROR;
}