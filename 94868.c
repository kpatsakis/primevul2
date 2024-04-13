zcolor_remap_one(
    i_ctx_t *           i_ctx_p,
    const ref *         pproc,
    gx_transfer_map *   pmap,
    const gs_gstate *    pgs,
    op_proc_t           finish_proc )
{
    os_ptr              op;

    /*
     * Detect the identity function, which is a common value for one or
     * more of these functions.
     */
    if (r_size(pproc) == 0) {
        gx_set_identity_transfer(pmap);
        /*
         * Even though we don't actually push anything on the e-stack, all
         * clients do, so we return o_push_estack in this case.  This is
         * needed so that clients' finishing procedures will get run.
         */
        return o_push_estack;
    }
    op = osp += 4;
    make_real(op - 3, 0);
    make_int(op - 2, transfer_map_size - 1);
    make_real(op - 1, 1);
    *op = *pproc;
    ++esp;
    make_struct(esp, imemory_space((gs_ref_memory_t *) pgs->memory),
                pmap);
    push_op_estack(finish_proc);
    push_op_estack(zfor_samples);
    return o_push_estack;
}
