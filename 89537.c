void vp9_free_context_buffers(VP9_COMMON *cm) {
  cm->free_mi(cm);
  free_seg_map(cm);
  vpx_free(cm->above_context);
  cm->above_context = NULL;
  vpx_free(cm->above_seg_context);
  cm->above_seg_context = NULL;
}
