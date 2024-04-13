int JBIG2MMRDecoder::get2DCode() {
  const CCITTCode *p;

  if (bufLen == 0) {
    buf = str->getChar() & 0xff;
    bufLen = 8;
    ++nBytesRead;
    p = &twoDimTab1[(buf >> 1) & 0x7f];
  } else if (bufLen == 8) {
    p = &twoDimTab1[(buf >> 1) & 0x7f];
  } else {
    p = &twoDimTab1[(buf << (7 - bufLen)) & 0x7f];
    if (p->bits < 0 || p->bits > (int)bufLen) {
      buf = (buf << 8) | (str->getChar() & 0xff);
      bufLen += 8;
      ++nBytesRead;
      p = &twoDimTab1[(buf >> (bufLen - 7)) & 0x7f];
    }
  }
  if (p->bits < 0) {
    error(str->getPos(), "Bad two dim code in JBIG2 MMR stream");
    return 0;
  }
  bufLen -= p->bits;
  return p->n;
}
