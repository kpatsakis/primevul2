njs_string_bytes_from_array_like(njs_vm_t *vm, njs_value_t *value)
{
    u_char              *p;
    int64_t             length;
    uint32_t            u32;
    njs_int_t           ret;
    njs_array_t         *array;
    njs_value_t         *octet, index, prop;
    njs_array_buffer_t  *buffer;

    array = NULL;
    buffer = NULL;

    switch (value->type) {
    case NJS_ARRAY:
        array = njs_array(value);
        length = array->length;
        break;

    case NJS_ARRAY_BUFFER:
    case NJS_TYPED_ARRAY:

        if (njs_is_typed_array(value)) {
            buffer = njs_typed_array(value)->buffer;

        } else {
            buffer = njs_array_buffer(value);
        }

        length = buffer->size;
        break;

    default:
        ret = njs_object_length(vm, value, &length);
        if (njs_slow_path(ret == NJS_ERROR)) {
            return ret;
        }
    }

    p = njs_string_alloc(vm, &vm->retval, length, 0);
    if (njs_slow_path(p == NULL)) {
        return NJS_ERROR;
    }

    if (array != NULL) {
        octet = array->start;

        while (length != 0) {
            ret = njs_value_to_uint32(vm, octet, &u32);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }

            *p++ = (u_char) u32;
            octet++;
            length--;
        }

    } else if (buffer != NULL) {
        memcpy(p, buffer->u.u8, length);

    } else {
        p += length - 1;

        while (length != 0) {
            njs_set_number(&index, length - 1);

            ret = njs_value_property(vm, value, &index, &prop);
            if (njs_slow_path(ret == NJS_ERROR)) {
                return ret;
            }

            ret = njs_value_to_uint32(vm, &prop, &u32);
            if (njs_slow_path(ret != NJS_OK)) {
                return ret;
            }

            *p-- = (u_char) u32;
            length--;
        }
    }

    return NJS_OK;
}