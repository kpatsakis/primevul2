njs_string_prototype_match(njs_vm_t *vm, njs_value_t *args, njs_uint_t nargs,
    njs_index_t unused)
{
    njs_str_t             string;
    njs_int_t             ret;
    njs_value_t           arguments[2];
    njs_regexp_pattern_t  *pattern;

    ret = njs_string_object_validate(vm, njs_argument(args, 0));
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

    arguments[1] = args[0];

    string.start = NULL;
    string.length = 0;

    if (nargs > 1) {

        if (njs_is_regexp(&args[1])) {
            pattern = njs_regexp_pattern(&args[1]);

            if (pattern->global) {
                return njs_string_match_multiple(vm, args, pattern);
            }

            /*
             * string.match(regexp) is the same as regexp.exec(string)
             * if the regexp has no global flag.
             */
            arguments[0] = args[1];

            goto match;
        }

        if (!njs_is_string(&args[1])) {
            if (!njs_is_undefined(&args[1])) {
                ret = njs_value_to_string(vm, &args[1], &args[1]);
                if (njs_slow_path(ret != NJS_OK)) {
                    return ret;
                }

                njs_string_get(&args[1], &string);
            }

        } else {
            njs_string_get(&args[1], &string);
        }

        /* A void value. */
    }

    ret = njs_regexp_create(vm, &arguments[0], string.start, string.length, 0);
    if (njs_slow_path(ret != NJS_OK)) {
        return ret;
    }

match:

    return njs_regexp_prototype_exec(vm, arguments, nargs, unused);
}