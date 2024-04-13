static int do_i2c_probe(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	int j;
	int addr = -1;
	int found = 0;
#if defined(CONFIG_SYS_I2C_NOPROBES)
	int k, skip;
	unsigned int bus = GET_BUS_NUM;
#endif	/* NOPROBES */
	int ret;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *bus, *dev;

	if (i2c_get_cur_bus(&bus))
		return CMD_RET_FAILURE;
#endif

	if (argc == 2)
		addr = simple_strtol(argv[1], 0, 16);

	puts ("Valid chip addresses:");
	for (j = 0; j < 128; j++) {
		if ((0 <= addr) && (j != addr))
			continue;

#if defined(CONFIG_SYS_I2C_NOPROBES)
		skip = 0;
		for (k = 0; k < ARRAY_SIZE(i2c_no_probes); k++) {
			if (COMPARE_BUS(bus, k) && COMPARE_ADDR(j, k)) {
				skip = 1;
				break;
			}
		}
		if (skip)
			continue;
#endif
#if CONFIG_IS_ENABLED(DM_I2C)
		ret = dm_i2c_probe(bus, j, 0, &dev);
#else
		ret = i2c_probe(j);
#endif
		if (ret == 0) {
			printf(" %02X", j);
			found++;
		}
	}
	putc ('\n');

#if defined(CONFIG_SYS_I2C_NOPROBES)
	puts ("Excluded chip addresses:");
	for (k = 0; k < ARRAY_SIZE(i2c_no_probes); k++) {
		if (COMPARE_BUS(bus,k))
			printf(" %02X", NO_PROBE_ADDR(k));
	}
	putc ('\n');
#endif

	return (0 == found);
}