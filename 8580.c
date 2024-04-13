njs_string_object_validate(njs_vm_t *vm, njs_value_t *object)
{
    njs_int_t  ret;

    if (njs_slow_path(njs_is_null_or_undefined(object))) {
        njs_type_error(vm, "cannot convert undefined to object");
        return NJS_ERROR;
    }

    if (njs_slow_path(!njs_is_string(object))) {
        ret = njs_value_to_string(vm, object, object);
        if (njs_slow_path(ret != NJS_OK)) {
            return ret;
        }
    }

    return NJS_OK;
}