fbFetch_a4r4g4b4 (const FbBits *bits, int x, int width, CARD32 *buffer, miIndexedPtr indexed)
{
    const CARD16 *pixel = (const CARD16 *)bits + x;
    const CARD16 *end = pixel + width;
    while (pixel < end) {
        CARD32  p = READ(pixel++);
        CARD32  r,g,b, a;

        a = ((p & 0xf000) | ((p & 0xf000) >> 4)) << 16;
        r = ((p & 0x0f00) | ((p & 0x0f00) >> 4)) << 12;
        g = ((p & 0x00f0) | ((p & 0x00f0) >> 4)) << 8;
        b = ((p & 0x000f) | ((p & 0x000f) << 4));
        WRITE(buffer++, (a | r | g | b));
    }
}
