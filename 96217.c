static u32 tg3_read32(struct tg3 *tp, u32 off)
{
	return readl(tp->regs + off);
}
