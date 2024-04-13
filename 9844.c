static int mod_i2c_mem(struct cmd_tbl *cmdtp, int incrflag, int flag, int argc,
		       char *const argv[])
{
	uint	chip;
	ulong	addr;
	int	alen;
	ulong	data;
	int	size = 1;
	int	nbytes;
	int ret;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
#endif

	if (argc != 3)
		return CMD_RET_USAGE;

	bootretry_reset_cmd_timeout();	/* got a good command to get here */
	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	chip = i2c_mm_last_chip;
	addr = i2c_mm_last_addr;
	alen = i2c_mm_last_alen;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/*
		 * New command specified.  Check for a size specification.
		 * Defaults to byte if no or incorrect specification.
		 */
		size = cmd_get_data_size(argv[0], 1);

		/*
		 * Chip is always specified.
		 */
		chip = hextoul(argv[1], NULL);

		/*
		 * Address is always specified.
		 */
		addr = hextoul(argv[2], NULL);
		alen = get_alen(argv[2], DEFAULT_ADDR_LEN);
		if (alen > 3)
			return CMD_RET_USAGE;
	}

#if CONFIG_IS_ENABLED(DM_I2C)
	ret = i2c_get_cur_bus_chip(chip, &dev);
	if (!ret && alen != -1)
		ret = i2c_set_chip_offset_len(dev, alen);
	if (ret)
		return i2c_report_err(ret, I2C_ERR_WRITE);
#endif

	/*
	 * Print the address, followed by value.  Then accept input for
	 * the next value.  A non-converted value exits.
	 */
	do {
		printf("%08lx:", addr);
#if CONFIG_IS_ENABLED(DM_I2C)
		ret = dm_i2c_read(dev, addr, (uchar *)&data, size);
#else
		ret = i2c_read(chip, addr, alen, (uchar *)&data, size);
#endif
		if (ret)
			return i2c_report_err(ret, I2C_ERR_READ);

		data = cpu_to_be32(data);
		if (size == 1)
			printf(" %02lx", (data >> 24) & 0x000000FF);
		else if (size == 2)
			printf(" %04lx", (data >> 16) & 0x0000FFFF);
		else
			printf(" %08lx", data);

		nbytes = cli_readline(" ? ");
		if (nbytes == 0) {
			/*
			 * <CR> pressed as only input, don't modify current
			 * location and move to next.
			 */
			if (incrflag)
				addr += size;
			nbytes = size;
			/* good enough to not time out */
			bootretry_reset_cmd_timeout();
		}
#ifdef CONFIG_BOOT_RETRY_TIME
		else if (nbytes == -2)
			break;	/* timed out, exit the command	*/
#endif
		else {
			char *endp;

			data = hextoul(console_buffer, &endp);
			if (size == 1)
				data = data << 24;
			else if (size == 2)
				data = data << 16;
			data = be32_to_cpu(data);
			nbytes = endp - console_buffer;
			if (nbytes) {
				/*
				 * good enough to not time out
				 */
				bootretry_reset_cmd_timeout();
#if CONFIG_IS_ENABLED(DM_I2C)
				ret = dm_i2c_write(dev, addr, (uchar *)&data,
						   size);
#else
				ret = i2c_write(chip, addr, alen,
						(uchar *)&data, size);
#endif
				if (ret)
					return i2c_report_err(ret,
							      I2C_ERR_WRITE);
#if CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS > 0
				udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
				if (incrflag)
					addr += size;
			}
		}
	} while (nbytes);

	i2c_mm_last_chip = chip;
	i2c_mm_last_addr = addr;
	i2c_mm_last_alen = alen;

	return 0;
}