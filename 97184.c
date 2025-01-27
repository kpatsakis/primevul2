static void crc32c_compute(u32 *crcp, const u64 *data, unsigned int len)
{
	unsigned int asm_len;

	asm_len = len & ~7U;
	if (asm_len) {
		crc32c_sparc64(crcp, data, asm_len);
		data += asm_len / 8;
		len -= asm_len;
	}
	if (len)
		*crcp = __crc32c_le(*crcp, (const unsigned char *) data, len);
}
