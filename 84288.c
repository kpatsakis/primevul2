static BROTLI_INLINE void TakeDistanceFromRingBuffer(BrotliState* s) {
  if (s->distance_code == 0) {
    --s->dist_rb_idx;
    s->distance_code = s->dist_rb[s->dist_rb_idx & 3];
  } else {
    int distance_code = s->distance_code << 1;
    /* kDistanceShortCodeIndexOffset has 2-bit values from LSB: */
    /* 3, 2, 1, 0, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 */
    const uint32_t kDistanceShortCodeIndexOffset = 0xaaafff1b;
    /* kDistanceShortCodeValueOffset has 2-bit values from LSB: */
    /*-0, 0,-0, 0,-1, 1,-2, 2,-3, 3,-1, 1,-2, 2,-3, 3 */
    const uint32_t kDistanceShortCodeValueOffset = 0xfa5fa500;
    int v = (s->dist_rb_idx +
        (int)(kDistanceShortCodeIndexOffset >> distance_code)) & 0x3;
    s->distance_code = s->dist_rb[v];
    v = (int)(kDistanceShortCodeValueOffset >> distance_code) & 0x3;
    if ((distance_code & 0x3) != 0) {
      s->distance_code += v;
    } else {
      s->distance_code -= v;
      if (s->distance_code <= 0) {
        /* A huge distance will cause a BROTLI_FAILURE() soon. */
        /* This is a little faster than failing here. */
        s->distance_code = 0x0fffffff;
      }
    }
  }
}
