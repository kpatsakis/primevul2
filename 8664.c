njs_array_iterator_next(njs_vm_t *vm, njs_value_t *iterator,
    njs_value_t *retval)
{
    int64_t               length;
    njs_int_t             ret;
    njs_array_t           *array, *entry;
    njs_typed_array_t     *tarray;
    const njs_value_t     *value;
    njs_array_iterator_t  *it;

    if (njs_slow_path(!njs_is_valid(njs_object_value(iterator)))) {
        return NJS_DECLINED;
    }

    it = njs_object_data(iterator);
    value = &njs_value_undefined;

    if (njs_is_fast_array(&it->target)) {
        array = njs_array(&it->target);
        length = array->length;

        if (it->next >= length) {
            goto release;
        }

        if (it->kind > NJS_ENUM_KEYS && njs_is_valid(&array->start[it->next])) {
            value = &array->start[it->next];
        }

    } else if (njs_is_typed_array(&it->target)) {
        tarray = njs_typed_array(&it->target);

        if (njs_slow_path(njs_is_detached_buffer(tarray->buffer))) {
            njs_type_error(vm, "detached buffer");
            return NJS_ERROR;
        }

        length = njs_typed_array_length(tarray);

        if (it->next >= length) {
            goto release;
        }

        if (it->kind > NJS_ENUM_KEYS) {
            njs_set_number(retval, njs_typed_array_prop(tarray, it->next));
            value = retval;
        }

    } else {
        ret = njs_object_length(vm, &it->target, &length);
        if (njs_slow_path(ret == NJS_ERROR)) {
            return ret;
        }

        if (it->next >= length) {
            goto release;
        }

        if (it->kind > NJS_ENUM_KEYS) {
            ret = njs_value_property_i64(vm, &it->target, it->next, retval);
            if (njs_slow_path(ret == NJS_ERROR)) {
                return ret;
            }

            value = njs_is_valid(retval) ? retval
                                         : &njs_value_undefined;
        }
    }

    switch (it->kind) {
    case NJS_ENUM_KEYS:
        njs_set_number(retval, it->next++);
        break;

    case NJS_ENUM_VALUES:
        it->next++;
        *retval = *value;
        break;

    case NJS_ENUM_BOTH:
        entry = njs_array_alloc(vm, 0, 2, 0);
        if (njs_slow_path(entry == NULL)) {
            return NJS_ERROR;
        }

        njs_set_number(&entry->start[0], it->next++);
        entry->start[1] = *value;

        njs_set_array(retval, entry);
        break;

    default:
        njs_internal_error(vm, "invalid enum kind");
        return NJS_ERROR;
    }

    return NJS_OK;

release:

    /* GC release it->target */
    njs_mp_free(vm->mem_pool, it);
    njs_set_invalid(njs_object_value(iterator));

    return NJS_DECLINED;
}