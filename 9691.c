njs_vm_clone(njs_vm_t *vm, njs_external_ptr_t external)
{
    njs_mp_t     *nmp;
    njs_vm_t     *nvm;
    njs_int_t    ret;
    njs_value_t  **global;

    njs_thread_log_debug("CLONE:");

    if (vm->options.interactive) {
        return NULL;
    }

    nmp = njs_mp_fast_create(2 * njs_pagesize(), 128, 512, 16);
    if (njs_slow_path(nmp == NULL)) {
        return NULL;
    }

    nvm = njs_mp_align(nmp, sizeof(njs_value_t), sizeof(njs_vm_t));
    if (njs_slow_path(nvm == NULL)) {
        goto fail;
    }

    *nvm = *vm;

    nvm->mem_pool = nmp;
    nvm->trace.data = nvm;
    nvm->external = external;

    ret = njs_vm_init(nvm);
    if (njs_slow_path(ret != NJS_OK)) {
        goto fail;
    }

    global = njs_scope_make(nvm, nvm->global_items);
    if (njs_slow_path(global == NULL)) {
        goto fail;
    }

    nvm->levels[NJS_LEVEL_GLOBAL] = global;

    njs_set_object(&nvm->global_value, &nvm->global_object);

    /* globalThis and this */
    njs_scope_value_set(nvm, njs_scope_global_this_index(), &nvm->global_value);

    nvm->levels[NJS_LEVEL_LOCAL] = NULL;

    return nvm;

fail:

    njs_mp_destroy(nmp);

    return NULL;
}