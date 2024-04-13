njs_string_instance_length(njs_vm_t *vm, njs_object_prop_t *prop,
    njs_value_t *value, njs_value_t *setval, njs_value_t *retval)
{
    size_t              size;
    uintptr_t           length;
    njs_object_t        *proto;
    njs_object_value_t  *ov;

    /*
     * This getter can be called for string primitive, String object,
     * String.prototype.  The zero should be returned for the latter case.
     */
    length = 0;

    if (njs_slow_path(njs_is_object(value))) {
        proto = njs_object(value);

        do {
            if (njs_fast_path(proto->type == NJS_OBJECT_VALUE)) {
                break;
            }

            proto = proto->__proto__;
        } while (proto != NULL);

        if (proto != NULL) {
            ov = (njs_object_value_t *) proto;
            value = &ov->value;
        }
    }

    if (njs_is_string(value)) {
        size = value->short_string.size;
        length = value->short_string.length;

        if (size == NJS_STRING_LONG) {
            size = value->long_string.size;
            length = value->long_string.data->length;
        }

        length = (length == 0) ? size : length;
    }

    njs_set_number(retval, length);

    njs_release(vm, value);

    return NJS_OK;
}