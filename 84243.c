BrotliResult BrotliDecompress(BrotliInput input, BrotliOutput output) {
  BrotliState s;
  BrotliResult result;
  BrotliStateInit(&s);
  result = BrotliDecompressStreaming(input, output, 1, &s);
  if (result == BROTLI_RESULT_NEEDS_MORE_INPUT) {
    /* Not ok: it didn't finish even though this is a non-streaming function. */
    result = BROTLI_FAILURE();
  }
  BrotliStateCleanup(&s);
  return result;
}
