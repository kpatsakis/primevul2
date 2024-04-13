static int parse_bitdepth_colorspace_sampling(
    BITSTREAM_PROFILE profile, struct vpx_read_bit_buffer *rb) {
 vpx_color_space_t color_space;
 if (profile >= PROFILE_2)
    rb->bit_offset += 1; // Bit-depth 10 or 12.
  color_space = (vpx_color_space_t)vpx_rb_read_literal(rb, 3);
 if (color_space != VPX_CS_SRGB) {
    rb->bit_offset += 1; // [16,235] (including xvycc) vs [0,255] range.
 if (profile == PROFILE_1 || profile == PROFILE_3) {
      rb->bit_offset += 2; // subsampling x/y.
      rb->bit_offset += 1; // unused.
 }
 } else {
 if (profile == PROFILE_1 || profile == PROFILE_3) {
      rb->bit_offset += 1; // unused
 } else {
 return 0;
 }
 }
 return 1;
}
