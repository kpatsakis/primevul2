static void tg3_dump_legacy_regs(struct tg3 *tp, u32 *regs)
{
	tg3_rd32_loop(tp, regs, TG3PCI_VENDOR, 0xb0);
	tg3_rd32_loop(tp, regs, MAILBOX_INTERRUPT_0, 0x200);
	tg3_rd32_loop(tp, regs, MAC_MODE, 0x4f0);
	tg3_rd32_loop(tp, regs, SNDDATAI_MODE, 0xe0);
	tg3_rd32_loop(tp, regs, SNDDATAC_MODE, 0x04);
	tg3_rd32_loop(tp, regs, SNDBDS_MODE, 0x80);
	tg3_rd32_loop(tp, regs, SNDBDI_MODE, 0x48);
	tg3_rd32_loop(tp, regs, SNDBDC_MODE, 0x04);
	tg3_rd32_loop(tp, regs, RCVLPC_MODE, 0x20);
	tg3_rd32_loop(tp, regs, RCVLPC_SELLST_BASE, 0x15c);
	tg3_rd32_loop(tp, regs, RCVDBDI_MODE, 0x0c);
	tg3_rd32_loop(tp, regs, RCVDBDI_JUMBO_BD, 0x3c);
	tg3_rd32_loop(tp, regs, RCVDBDI_BD_PROD_IDX_0, 0x44);
	tg3_rd32_loop(tp, regs, RCVDCC_MODE, 0x04);
	tg3_rd32_loop(tp, regs, RCVBDI_MODE, 0x20);
	tg3_rd32_loop(tp, regs, RCVCC_MODE, 0x14);
	tg3_rd32_loop(tp, regs, RCVLSC_MODE, 0x08);
	tg3_rd32_loop(tp, regs, MBFREE_MODE, 0x08);
	tg3_rd32_loop(tp, regs, HOSTCC_MODE, 0x100);

	if (tg3_flag(tp, SUPPORT_MSIX))
		tg3_rd32_loop(tp, regs, HOSTCC_RXCOL_TICKS_VEC1, 0x180);

	tg3_rd32_loop(tp, regs, MEMARB_MODE, 0x10);
	tg3_rd32_loop(tp, regs, BUFMGR_MODE, 0x58);
	tg3_rd32_loop(tp, regs, RDMAC_MODE, 0x08);
	tg3_rd32_loop(tp, regs, WDMAC_MODE, 0x08);
	tg3_rd32_loop(tp, regs, RX_CPU_MODE, 0x04);
	tg3_rd32_loop(tp, regs, RX_CPU_STATE, 0x04);
	tg3_rd32_loop(tp, regs, RX_CPU_PGMCTR, 0x04);
	tg3_rd32_loop(tp, regs, RX_CPU_HWBKPT, 0x04);

	if (!tg3_flag(tp, 5705_PLUS)) {
		tg3_rd32_loop(tp, regs, TX_CPU_MODE, 0x04);
		tg3_rd32_loop(tp, regs, TX_CPU_STATE, 0x04);
		tg3_rd32_loop(tp, regs, TX_CPU_PGMCTR, 0x04);
	}

	tg3_rd32_loop(tp, regs, GRCMBOX_INTERRUPT_0, 0x110);
	tg3_rd32_loop(tp, regs, FTQ_RESET, 0x120);
	tg3_rd32_loop(tp, regs, MSGINT_MODE, 0x0c);
	tg3_rd32_loop(tp, regs, DMAC_MODE, 0x04);
	tg3_rd32_loop(tp, regs, GRC_MODE, 0x4c);

	if (tg3_flag(tp, NVRAM))
		tg3_rd32_loop(tp, regs, NVRAM_CMD, 0x24);
}
