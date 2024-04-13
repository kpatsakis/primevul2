INST_HANDLER (bset) {	// BSET s
	int s = (buf[0] >> 4) & 0x7;
	ESIL_A ("%d,1,<<,sreg,|=,", s);
}
