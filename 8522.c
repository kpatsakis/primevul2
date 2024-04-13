njs_string_prototype_char_code_at(njs_vm_t *vm, njs_value_t *args,
    njs_uint_t nargs, njs_index_t unused)
{
    double                num;
    size_t                length;
    int64_t               index;
    uint32_t              code;
    njs_int_t             ret;
    const u_char          *start, *end;
    njs_string_prop_t     string;
    njs_unicode_decode_t  ctx;

    ret = njs_string_object_validate(vm, njs_argument(args, 0));
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    length = njs_string_prop(&string, njs_argument(args, 0));

    ret = njs_value_to_integer(vm, njs_arg(args, nargs, 1), &index);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    if (njs_slow_path(index < 0 || index >= (int64_t) length)) {
        num = NAN;
        goto done;
    }

    if (length == string.size) {
        /* Byte or ASCII string. */
        code = string.start[index];

    } else {
        njs_utf8_decode_init(&ctx);

        /* UTF-8 string. */
        end = string.start + string.size;
        start = njs_string_offset(string.start, end, index);
        code = njs_utf8_decode(&ctx, &start, end);
    }

    num = code;

done:

    njs_set_number(&vm->retval, num);

    return NJS_OK;
}