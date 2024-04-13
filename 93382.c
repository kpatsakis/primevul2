static int load_segment_descriptor(struct x86_emulate_ctxt *ctxt,
 				   u16 selector, int seg)
 {
 	u8 cpl = ctxt->ops->cpl(ctxt);

	/*
	 * None of MOV, POP and LSS can load a NULL selector in CPL=3, but
	 * they can load it at CPL<3 (Intel's manual says only LSS can,
	 * but it's wrong).
	 *
	 * However, the Intel manual says that putting IST=1/DPL=3 in
	 * an interrupt gate will result in SS=3 (the AMD manual instead
	 * says it doesn't), so allow SS=3 in __load_segment_descriptor
	 * and only forbid it here.
	 */
	if (seg == VCPU_SREG_SS && selector == 3 &&
	    ctxt->mode == X86EMUL_MODE_PROT64)
		return emulate_exception(ctxt, GP_VECTOR, 0, true);

 	return __load_segment_descriptor(ctxt, selector, seg, cpl,
 					 X86_TRANSFER_NONE, NULL);
 }
