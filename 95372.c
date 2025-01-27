JBIG2Bitmap::JBIG2Bitmap(Guint segNumA, int wA, int hA):
  JBIG2Segment(segNumA)
{
  w = wA;
  h = hA;
  line = (wA + 7) >> 3;

  if (w <= 0 || h <= 0 || line <= 0 || h >= (INT_MAX - 1) / line) {
    error(errSyntaxError, -1, "invalid width/height");
    data = NULL;
    return;
  }
  data = (Guchar *)gmalloc_checkoverflow(h * line + 1);
  if (data != NULL) {
    data[h * line] = 0;
  }
}
