void ass_stripe_pack_c(uint8_t *dst, ptrdiff_t dst_stride, const int16_t *src,
                       uintptr_t width, uintptr_t height)
{
    for (uintptr_t x = 0; x < width; x += STRIPE_WIDTH) {
        uint8_t *ptr = dst;
        for (uintptr_t y = 0; y < height; ++y) {
            const int16_t *dither = dither_line + (y & 1) * STRIPE_WIDTH;
            for (int k = 0; k < STRIPE_WIDTH; ++k)
                ptr[k] = (uint16_t)(src[k] - (src[k] >> 8) + dither[k]) >> 6;
            ptr += dst_stride;
            src += STRIPE_WIDTH;
        }
        dst += STRIPE_WIDTH;
    }
    uintptr_t left = dst_stride - ((width + STRIPE_MASK) & ~STRIPE_MASK);
    for (uintptr_t y = 0; y < height; ++y) {
        for (uintptr_t x = 0; x < left; ++x)
            dst[x] = 0;
        dst += dst_stride;
    }
}
