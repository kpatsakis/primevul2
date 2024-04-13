INST_HANDLER (brbx) {	// BRBC s, k
	int s = buf[0] & 0x7;
	op->jump = op->addr
		+ ((((buf[1] & 0x03) << 6) | ((buf[0] & 0xf8) >> 2))
			| (buf[1] & 0x2 ? ~((int) 0x7f) : 0))
		+ 2;
	op->fail = op->addr + op->size;
	op->cycles = 1;	// XXX: This is a bug, because depends on eval state,
	ESIL_A ("%d,1,<<,sreg,&,", s);				// SREG(s)
	ESIL_A (buf[1] & 0x4
			? "!,"		// BRBC => branch if cleared
			: "!,!,");	// BRBS => branch if set
	ESIL_A ("?{,%"PFMT64d",pc,=,},", op->jump);	// ?true => jmp
}
