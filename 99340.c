INST_HANDLER (std) {	// ST Y, Rr	ST Z, Rr
	ESIL_A ("r%d,", ((buf[1] & 1) << 4) | ((buf[0] >> 4) & 0xf));
	__generic_ld_st (
		op, "ram",
		buf[0] & 0x8 ? 'y' : 'z',	// index register Y/Z
		0,				// no use RAMP* registers
		!(buf[1] & 0x10)
			? 0			// no increment
			: buf[0] & 0x1
				? 1		// post incremented
				: -1,		// pre decremented
		!(buf[1] & 0x10)
			? (buf[1] & 0x20)	// offset
			| ((buf[1] & 0xc) << 1)
			| (buf[0] & 0x7)
			: 0,			// no offset
		1);				// load operation (!st)
}
