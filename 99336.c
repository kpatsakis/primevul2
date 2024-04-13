INST_HANDLER (ldd) {	// LD Rd, Y	LD Rd, Z
	int offset = (buf[1] & 0x20)
			| ((buf[1] & 0xc) << 1)
			| (buf[0] & 0x7);
	__generic_ld_st (
		op, "ram",
		buf[0] & 0x8 ? 'y' : 'z',	// index register Y/Z
		0,				// no use RAMP* registers
		!(buf[1] & 0x10)
			? 0			// no increment
			: buf[0] & 0x1
				? 1		// post incremented
				: -1,		// pre decremented
		!(buf[1] & 0x10) ? offset : 0,	// offset or not offset
		0);				// load operation (!st)
	ESIL_A ("r%d,=,", ((buf[1] & 1) << 4) | ((buf[0] >> 4) & 0xf));
	op->cycles =
		(buf[1] & 0x10) == 0
			? (!offset ? 1 : 3)		// LDD
			: (buf[0] & 0x3) == 0
				? 1			// LD Rd, X
				: (buf[0] & 0x3) == 1
					? 2		// LD Rd, X+
					: 3;		// LD Rd, -X
	if (!STR_BEGINS (cpu->model, "ATxmega") && op->cycles > 1) {
		op->cycles--;
	}
}
