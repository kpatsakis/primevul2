static int BROTLI_NOINLINE BrotliAllocateRingBuffer(BrotliState* s,
    BrotliBitReader* br) {
  /* We need the slack region for the following reasons:
      - doing up to two 16-byte copies for fast backward copying
      - inserting transformed dictionary word (5 prefix + 24 base + 8 suffix) */
  static const int kRingBufferWriteAheadSlack = 42;
  int is_last = s->is_last_metablock;
  s->ringbuffer_size = 1 << s->window_bits;

  if (s->is_uncompressed) {
    int next_block_header = BrotliPeekByte(br,
        (size_t)s->meta_block_remaining_len);
    if (next_block_header != -1) { /* Peek succeeded */
      if ((next_block_header & 3) == 3) { /* ISLAST and ISEMPTY */
        is_last = 1;
      }
    }
  }

  /* We need at least 2 bytes of ring buffer size to get the last two
     bytes for context from there */
  if (is_last) {
    while (s->ringbuffer_size >= s->meta_block_remaining_len * 2
        && s->ringbuffer_size > 32) {
      s->ringbuffer_size >>= 1;
    }
  }

  /* But make it fit the custom dictionary if there is one. */
  while (s->ringbuffer_size < s->custom_dict_size) {
    s->ringbuffer_size <<= 1;
  }

  s->ringbuffer_mask = s->ringbuffer_size - 1;
  s->ringbuffer = (uint8_t*)BROTLI_ALLOC(s, (size_t)(s->ringbuffer_size +
      kRingBufferWriteAheadSlack + kBrotliMaxDictionaryWordLength));
  if (s->ringbuffer == 0) {
    return 0;
  }
  s->ringbuffer_end = s->ringbuffer + s->ringbuffer_size;
  s->ringbuffer[s->ringbuffer_size - 2] = 0;
  s->ringbuffer[s->ringbuffer_size - 1] = 0;
  if (s->custom_dict) {
    memcpy(&s->ringbuffer[(-s->custom_dict_size) & s->ringbuffer_mask],
                          s->custom_dict, (size_t)s->custom_dict_size);
  }

  return 1;
}
