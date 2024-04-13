ringbuffer_read(uint8_t **dst, size_t dstlen, struct ringbuffer *buf)
{
  int remaining;

  *dst = buf->buffer + buf->read_pos;

  if (buf->read_avail == 0 || dstlen == 0)
    return 0;

  remaining = buf->size - buf->read_pos;

  // The number of bytes we will return will be MIN(dstlen, remaining, read_avail)
  if (dstlen > remaining)
    dstlen = remaining;
  if (dstlen > buf->read_avail)
    dstlen = buf->read_avail;

  buf->read_pos = (buf->read_pos + dstlen) % buf->size;

  buf->write_avail += dstlen;
  buf->read_avail -= dstlen;

  return dstlen;
}