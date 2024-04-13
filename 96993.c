static int collect_cpu_info_early(struct ucode_cpu_info *uci)
{
	unsigned int val[2];
	u8 x86, x86_model;
	struct cpu_signature csig;
	unsigned int eax, ebx, ecx, edx;

	csig.sig = 0;
	csig.pf = 0;
	csig.rev = 0;

	memset(uci, 0, sizeof(*uci));

	eax = 0x00000001;
	ecx = 0;
	native_cpuid(&eax, &ebx, &ecx, &edx);
	csig.sig = eax;

	x86 = get_x86_family(csig.sig);
	x86_model = get_x86_model(csig.sig);

	if ((x86_model >= 5) || (x86 > 6)) {
		/* get processor flags from MSR 0x17 */
		native_rdmsr(MSR_IA32_PLATFORM_ID, val[0], val[1]);
		csig.pf = 1 << ((val[1] >> 18) & 7);
	}
	native_wrmsr(MSR_IA32_UCODE_REV, 0, 0);

	/* As documented in the SDM: Do a CPUID 1 here */
	sync_core();

	/* get the current revision from MSR 0x8B */
	native_rdmsr(MSR_IA32_UCODE_REV, val[0], val[1]);

	csig.rev = val[1];

	uci->cpu_sig = csig;
	uci->valid = 1;

	return 0;
}
