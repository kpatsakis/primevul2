njs_string_bytes_from_string(njs_vm_t *vm, const njs_value_t *string,
    const njs_value_t *encoding)
{
    njs_str_t  enc, str;

    if (!njs_is_string(encoding)) {
        njs_type_error(vm, "\"encoding\" must be a string");
        return NJS_ERROR;
    }

    njs_string_get(encoding, &enc);
    njs_string_get(string, &str);

    if (enc.length == 3 && memcmp(enc.start, "hex", 3) == 0) {
        return njs_string_decode_hex(vm, &vm->retval, &str);

    } else if (enc.length == 6 && memcmp(enc.start, "base64", 6) == 0) {
        return njs_string_decode_base64(vm, &vm->retval, &str);

    } else if (enc.length == 9 && memcmp(enc.start, "base64url", 9) == 0) {
        return njs_string_decode_base64url(vm, &vm->retval, &str);
    }

    njs_type_error(vm, "Unknown encoding: \"%V\"", &enc);

    return NJS_ERROR;
}