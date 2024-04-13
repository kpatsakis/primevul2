static Register parseReg(RAsm *a, const char *str, size_t *pos, ut32 *type) {
	int i;
	const char *regs[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "eip", NULL };
	const char *regsext[] = { "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d", NULL };
	const char *regs8[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", NULL };
	const char *regs16[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", NULL };
	const char *regs64[] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "rip", NULL};
	const char *regs64ext[] = { "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", NULL };
	const char *sregs[] = { "es", "cs", "ss", "ds", "fs", "gs", NULL};

	size_t nextpos, length;
	const char *token;
	getToken (str, pos, &nextpos);
	token = str + *pos;
	length = nextpos - *pos;
	*pos = nextpos;

	if (length == 3 && token[0] == 'e') {
		for (i = 0; regs[i]; i++) {
			if (!r_str_ncasecmp (regs[i], token, length)) {
				*type = (OT_GPREG & OT_REG (i)) | OT_DWORD;
				return i;
			}
		}
	}
	if (length == 2 && (token[1] == 'l' || token[1] == 'h')) {
		for (i = 0; regs8[i]; i++) {
			if (!r_str_ncasecmp (regs8[i], token, length)) {
				*type = (OT_GPREG & OT_REG (i)) | OT_BYTE;
				return i;
			}
		}
	}
	if (length == 2) {
		for (i = 0; regs16[i]; i++) {
			if (!r_str_ncasecmp (regs16[i], token, length)) {
				*type = (OT_GPREG & OT_REG (i)) | OT_WORD;
				return i;
			}
		}
		for (i = 0; sregs[i]; i++) {
			if (!r_str_ncasecmp (sregs[i], token, length)) {
				*type = (OT_SEGMENTREG & OT_REG (i)) | OT_WORD;
				return i;
			}
		}
	}
	if (token[0] == 'r') {
		for (i = 0; regs64[i]; i++) {
			if (!r_str_ncasecmp (regs64[i], token, length)) {
				*type = (OT_GPREG & OT_REG (i)) | OT_QWORD;
				a->bits = 64;
				return i;
			}
		}
		for (i = 0; regs64ext[i]; i++) {
			if (!r_str_ncasecmp (regs64ext[i], token, length)) {
				*type = (OT_GPREG & OT_REG (i)) | OT_QWORD;
				a->bits = 64;
				return i + 9;
			}
		}
		for (i = 0; regsext[i]; i++) {
			if (!r_str_ncasecmp (regsext[i], token, length)) {
				*type = (OT_GPREG & OT_REG (i)) | OT_DWORD;
				if (a->bits < 32) {
					a->bits = 32;
				}
				return i + 9;
			}
		}
	}

	if (!r_str_ncasecmp ("st", token, 2)) {
		*type = (OT_FPUREG & ~OT_REGALL);
		*pos = 3;
	}
	if (!r_str_ncasecmp ("mm", token, 2)) {
		*type = (OT_MMXREG & ~OT_REGALL);
		*pos = 3;
	}
	if (!r_str_ncasecmp ("xmm", token, 3)) {
		*type = (OT_XMMREG & ~OT_REGALL);
		*pos = 4;
	}

	if (*type & (OT_FPUREG | OT_MMXREG | OT_XMMREG) & ~OT_REGALL) {
		Register reg = X86R_UNDEFINED;

		if (getToken (str, pos, &nextpos) == TT_SPECIAL && str[*pos] == '(') {
			*pos = nextpos;
		}

		if (getToken (str, pos, &nextpos) != TT_NUMBER ||
				(reg = getnum (a, str + *pos)) > 7) {
			if ((int)reg > 15) {
				eprintf ("Too large register index!\n");
				return X86R_UNDEFINED;
			} else {
				reg -= 8;
			}
		}

		*pos = nextpos;

		if (getToken (str, pos, &nextpos) == TT_SPECIAL && str[*pos] == ')') {
			*pos = nextpos;
		}
		if (reg > 7) {
			eprintf ("Too large register index!\n");
			return X86R_UNDEFINED;
		}
		*type |= (OT_REG (reg) & ~OT_REGTYPE);
		return reg;
	}

	return X86R_UNDEFINED;
}
