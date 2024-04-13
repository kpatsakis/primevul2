njs_array_iterator_create(njs_vm_t *vm, const njs_value_t *target,
    njs_value_t *retval, njs_object_enum_t kind)
{
    njs_object_value_t    *iterator;
    njs_array_iterator_t  *it;

    iterator = njs_object_value_alloc(vm, NJS_OBJ_TYPE_ARRAY_ITERATOR, 0, NULL);
    if (njs_slow_path(iterator == NULL)) {
        njs_memory_error(vm);
        return NJS_ERROR;
    }

    it = njs_mp_alloc(vm->mem_pool, sizeof(njs_array_iterator_t));
    if (njs_slow_path(it == NULL)) {
        njs_memory_error(vm);
        return NJS_ERROR;
    }

    /* GC retain it->target */
    it->target = *target;
    it->next = 0;
    it->kind = kind;

    njs_set_data(&iterator->value, it, NJS_DATA_TAG_ARRAY_ITERATOR);
    njs_set_object_value(retval, iterator);

    return NJS_OK;
}