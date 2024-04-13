static int handle_init(struct fuse* fuse, struct fuse_handler* handler,
 const struct fuse_in_header* hdr, const struct fuse_init_in* req)
{
 struct fuse_init_out out;
 size_t fuse_struct_size;

    TRACE("[%d] INIT ver=%d.%d maxread=%d flags=%x\n",
            handler->token, req->major, req->minor, req->max_readahead, req->flags);

 /* Kernel 2.6.16 is the first stable kernel with struct fuse_init_out
     * defined (fuse version 7.6). The structure is the same from 7.6 through
     * 7.22. Beginning with 7.23, the structure increased in size and added
     * new parameters.
     */
 if (req->major != FUSE_KERNEL_VERSION || req->minor < 6) {
        ERROR("Fuse kernel version mismatch: Kernel version %d.%d, Expected at least %d.6",
              req->major, req->minor, FUSE_KERNEL_VERSION);
 return -1;
 }

 /* We limit ourselves to 15 because we don't handle BATCH_FORGET yet */
    out.minor = MIN(req->minor, 15);
    fuse_struct_size = sizeof(out);
#if defined(FUSE_COMPAT_22_INIT_OUT_SIZE)
 /* FUSE_KERNEL_VERSION >= 23. */

 /* If the kernel only works on minor revs older than or equal to 22,
     * then use the older structure size since this code only uses the 7.22
     * version of the structure. */
 if (req->minor <= 22) {
        fuse_struct_size = FUSE_COMPAT_22_INIT_OUT_SIZE;
 }
#endif

    out.major = FUSE_KERNEL_VERSION;
    out.max_readahead = req->max_readahead;
    out.flags = FUSE_ATOMIC_O_TRUNC | FUSE_BIG_WRITES;
    out.max_background = 32;
    out.congestion_threshold = 32;
    out.max_write = MAX_WRITE;
    fuse_reply(fuse, hdr->unique, &out, fuse_struct_size);
 return NO_STATUS;
}
