void CairoOutputDev::popTransparencyGroup() {
  /* pop color space */
  ColorSpaceStack *css = groupColorSpaceStack;
  if (css->knockout) {
    knockoutCount--;
    if (!knockoutCount) {
      /* we don't need to track the shape anymore because
       * we are not above any knockout groups */
      cairo_destroy(cairo_shape);
      cairo_shape = NULL;
    }
  }
  groupColorSpaceStack = css->next;
  delete css;
}
