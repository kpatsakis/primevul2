void vp9_free_ref_frame_buffers(BufferPool *pool) {
 int i;

 for (i = 0; i < FRAME_BUFFERS; ++i) {
 if (pool->frame_bufs[i].ref_count > 0 &&
        pool->frame_bufs[i].raw_frame_buffer.data != NULL) {
      pool->release_fb_cb(pool->cb_priv, &pool->frame_bufs[i].raw_frame_buffer);
      pool->frame_bufs[i].ref_count = 0;
 }
    vpx_free(pool->frame_bufs[i].mvs);
    pool->frame_bufs[i].mvs = NULL;
    vpx_free_frame_buffer(&pool->frame_bufs[i].buf);
 }
}
