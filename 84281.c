static BROTLI_NOINLINE int SafeDecodeSymbol(const HuffmanCode* table,
                                            BrotliBitReader* br,
                                            uint32_t* result) {
  uint32_t val;
  uint32_t available_bits = BrotliGetAvailableBits(br);
  if (available_bits == 0) {
    if (table->bits == 0) {
      *result = table->value;
      return 1;
    }
    return 0; /* No valid bits at all. */
  }
  val = (uint32_t)BrotliGetBitsUnmasked(br);
  table += val & HUFFMAN_TABLE_MASK;
  if (table->bits <= HUFFMAN_TABLE_BITS) {
    if (table->bits <= available_bits) {
      BrotliDropBits(br, table->bits);
      *result = table->value;
      return 1;
    } else {
      return 0; /* Not enough bits for the first level. */
    }
  }
  if (available_bits <= HUFFMAN_TABLE_BITS) {
    return 0; /* Not enough bits to move to the second level. */
  }

  /* Speculatively drop HUFFMAN_TABLE_BITS. */
  val = (val & BitMask(table->bits)) >> HUFFMAN_TABLE_BITS;
  available_bits -= HUFFMAN_TABLE_BITS;
  table += table->value + val;
  if (available_bits < table->bits) {
    return 0; /* Not enough bits for the second level. */
  }

  BrotliDropBits(br, HUFFMAN_TABLE_BITS + table->bits);
  *result = table->value;
  return 1;
}
