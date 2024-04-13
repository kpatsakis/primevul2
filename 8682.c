static int next_chunk_len(struct mctp_serial *dev)
{
	int i;

	/* either we have no bytes to send ... */
	if (dev->txpos == dev->txlen)
		return 0;

	/* ... or the next byte to send is an escaped byte; requiring a
	 * single-byte chunk...
	 */
	if (needs_escape(dev->txbuf[dev->txpos]))
		return 1;

	/* ... or we have one or more bytes up to the next escape - this chunk
	 * will be those non-escaped bytes, and does not include the escaped
	 * byte.
	 */
	for (i = 1; i + dev->txpos + 1 < dev->txlen; i++) {
		if (needs_escape(dev->txbuf[dev->txpos + i + 1]))
			break;
	}

	return i;
}