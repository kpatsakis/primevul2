static void update_read_synchronize(rdpUpdate* update, wStream* s)
{
	Stream_Seek_UINT16(s); /* pad2Octets (2 bytes) */
	/**
	 * The Synchronize Update is an artifact from the
	 * T.128 protocol and should be ignored.
	 */
}
