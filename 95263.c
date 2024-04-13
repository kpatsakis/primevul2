int dmarc_store_data(header_line *hdr) {
  /* No debug output because would change every test debug output */
  if (dmarc_disable_verify != TRUE)
    from_header = hdr;
  return OK;
}
