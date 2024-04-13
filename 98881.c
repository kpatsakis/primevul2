static int opnot(RAsm *a, ut8 * data, const Opcode *op) {
	int l = 0;

	if (op->operands[0].reg == X86R_UNDEFINED)  {
		return -1;
	}

	int size = op->operands[0].type & ALL_SIZE;
	if (op->operands[0].explicit_size) {
		size = op->operands[0].dest_size;
	}
	int rex = 1 << 6;
	bool use_rex = false;
	if (size & OT_QWORD) {			//W field
		use_rex = true;
		rex |= 1 << 3;
	}
	if (op->operands[0].extended) {		//B field
		use_rex = true;
		rex |= 1;
	}

	if (use_rex) {
		data[l++] = rex;
	}
	data[l++] = 0xf7;
	data[l++] = 0xd0 | op->operands[0].reg;

	return l;
}
