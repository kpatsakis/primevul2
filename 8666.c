njs_object_iterate(njs_vm_t *vm, njs_iterator_args_t *args,
    njs_iterator_handler_t handler)
{
    double              idx;
    int64_t             length, i, from, to;
    njs_int_t           ret;
    njs_array_t         *array, *keys;
    njs_value_t         *value, *entry, prop, character, string_obj;
    const u_char        *p, *end, *pos;
    njs_string_prop_t   string_prop;
    njs_object_value_t  *object;

    value = args->value;
    from = args->from;
    to = args->to;

    if (njs_is_array(value)) {
        array = njs_array(value);

        for (; from < to; from++) {
            if (njs_slow_path(!array->object.fast_array)) {
                goto process_object;
            }

            if (njs_fast_path(from < array->length
                              && njs_is_valid(&array->start[from])))
            {
                ret = handler(vm, args, &array->start[from], from);

            } else {
                entry = njs_value_arg(&njs_value_invalid);
                ret = njs_value_property_i64(vm, value, from, &prop);
                if (njs_slow_path(ret != NJS_DECLINED)) {
                    if (ret == NJS_ERROR) {
                        return NJS_ERROR;
                    }

                    entry = &prop;
                }

                ret = handler(vm, args, entry, from);
            }

            if (njs_slow_path(ret != NJS_OK)) {
                if (ret == NJS_DONE) {
                    return NJS_DONE;
                }

                return NJS_ERROR;
            }
        }

        return NJS_OK;
    }

    if (njs_is_string(value) || njs_is_object_string(value)) {

        if (njs_is_string(value)) {
            object = njs_object_value_alloc(vm, NJS_OBJ_TYPE_STRING, 0, value);
            if (njs_slow_path(object == NULL)) {
                return NJS_ERROR;
            }

            njs_set_object_value(&string_obj, object);

            args->value = &string_obj;
        }
        else {
            value = njs_object_value(value);
        }

        length = njs_string_prop(&string_prop, value);

        p = string_prop.start;
        end = p + string_prop.size;

        if ((size_t) length == string_prop.size) {
            /* Byte or ASCII string. */

            for (i = from; i < to; i++) {
                /* This cannot fail. */
                (void) njs_string_new(vm, &character, p + i, 1, 1);

                ret = handler(vm, args, &character, i);
                if (njs_slow_path(ret != NJS_OK)) {
                    if (ret == NJS_DONE) {
                        return NJS_DONE;
                    }

                    return NJS_ERROR;
                }
            }

        } else {
            /* UTF-8 string. */

            for (i = from; i < to; i++) {
                pos = njs_utf8_next(p, end);

                /* This cannot fail. */
                (void) njs_string_new(vm, &character, p, pos - p, 1);

                ret = handler(vm, args, &character, i);
                if (njs_slow_path(ret != NJS_OK)) {
                    if (ret == NJS_DONE) {
                        return NJS_DONE;
                    }

                    return NJS_ERROR;
                }

                p = pos;
            }
        }

        return NJS_OK;
    }

    if (!njs_is_object(value)) {
        return NJS_OK;
    }

process_object:

    if (!njs_fast_object(to - from)) {
        keys = njs_array_indices(vm, value);
        if (njs_slow_path(keys == NULL)) {
            return NJS_ERROR;
        }

        for (i = 0; i < keys->length; i++) {
            idx = njs_string_to_index(&keys->start[i]);

            if (idx < from || idx >= to) {
                continue;
            }

            ret = njs_iterator_object_handler(vm, handler, args, &keys->start[i],
                                           idx);
            if (njs_slow_path(ret != NJS_OK)) {
                njs_array_destroy(vm, keys);
                return ret;
            }
        }

        njs_array_destroy(vm, keys);

        return NJS_OK;
    }

    for (i = from; i < to; i++) {
        ret = njs_iterator_object_handler(vm, handler, args, NULL, i);
        if (njs_slow_path(ret != NJS_OK)) {
            return ret;
        }
    }

    return NJS_OK;
}