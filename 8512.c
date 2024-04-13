njs_string_prototype_search(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    size_t                c;
    njs_int_t             ret, index;
    njs_uint_t            n;
    njs_value_t           *value;
    njs_string_prop_t     string;
    njs_regexp_pattern_t  *pattern;

    ret = njs_string_object_validate(vm, njs_argument(args, 0));
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    index = 0;

    if (nargs > 1) {
        value = njs_argument(args, 1);

        switch (value->type) {

        case NJS_REGEXP:
            pattern = njs_regexp_pattern(value);
            break;

        case NJS_UNDEFINED:
            goto done;

        default:
            if (njs_slow_path(!njs_is_string(value))) {
                ret = njs_value_to_string(vm, value, value);
                if (njs_slow_path(ret != NJS_OK)) {
                    return ret;
                }
            }

            (void) njs_string_prop(&string, value);

            if (string.size != 0) {
                pattern = njs_regexp_pattern_create(vm, string.start,
                                                    string.size, 0);
                if (njs_slow_path(pattern == NULL)) {
                    return NJS_ERROR;
                }

                break;
            }

            goto done;
        }

        index = -1;

        (void) njs_string_prop(&string, &args[0]);

        n = (string.length != 0);

        if (njs_regex_is_valid(&pattern->regex[n])) {
            ret = njs_regexp_match(vm, &pattern->regex[n], string.start,
                                   0, string.size, vm->single_match_data);
            if (ret >= 0) {
                c = njs_regex_capture(vm->single_match_data, 0);
                index = njs_string_index(&string, c);

            } else if (ret == NJS_ERROR) {
                return NJS_ERROR;
            }
        }
    }

done:

    njs_set_number(&vm->retval, index);

    return NJS_OK;
}