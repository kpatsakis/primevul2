njs_string_prototype_to_upper_case(njs_vm_t *vm, njs_value_t *args,
    njs_uint_t nargs, njs_index_t unused)
{
    size_t             size, length;
    u_char             *p;
    uint32_t           code;
    njs_int_t          ret;
    const u_char       *s, *end;
    njs_string_prop_t  string;

    ret = njs_string_object_validate(vm, njs_argument(args, 0));
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    (void) njs_string_prop(&string, njs_argument(args, 0));

    if (njs_is_byte_or_ascii_string(&string)) {

        p = njs_string_alloc(vm, &vm->retval, string.size, string.length);
        if (njs_slow_path(p == NULL)) {
            return NJS_ERROR;
        }

        s = string.start;
        size = string.size;

        while (size != 0) {
            *p++ = njs_upper_case(*s++);
            size--;
        }

    } else {
        /* UTF-8 string. */
        s = string.start;
        end = s + string.size;
        length = string.length;

        size = 0;

        while (length != 0) {
            code = njs_utf8_upper_case(&s, end);
            size += njs_utf8_size(code);
            length--;
        }

        p = njs_string_alloc(vm, &vm->retval, size, string.length);
        if (njs_slow_path(p == NULL)) {
            return NJS_ERROR;
        }

        s = string.start;
        length = string.length;

        while (length != 0) {
            code = njs_utf8_upper_case(&s, end);
            p = njs_utf8_encode(p, code);
            length--;
        }
    }

    return NJS_OK;
}