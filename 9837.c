static int do_sdram(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	enum { unknown, EDO, SDRAM, DDR, DDR2, DDR3, DDR4 } type;

	uint	chip;
	u_char	data[128];
	u_char	cksum;
	int	j, ret;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
#endif

	static const char *decode_CAS_DDR2[] = {
		" TBD", " 6", " 5", " 4", " 3", " 2", " TBD", " TBD"
	};

	static const char *decode_CAS_default[] = {
		" TBD", " 7", " 6", " 5", " 4", " 3", " 2", " 1"
	};

	static const char *decode_CS_WE_default[] = {
		" TBD", " 6", " 5", " 4", " 3", " 2", " 1", " 0"
	};

	static const char *decode_byte21_default[] = {
		"  TBD (bit 7)\n",
		"  Redundant row address\n",
		"  Differential clock input\n",
		"  Registerd DQMB inputs\n",
		"  Buffered DQMB inputs\n",
		"  On-card PLL\n",
		"  Registered address/control lines\n",
		"  Buffered address/control lines\n"
	};

	static const char *decode_byte22_DDR2[] = {
		"  TBD (bit 7)\n",
		"  TBD (bit 6)\n",
		"  TBD (bit 5)\n",
		"  TBD (bit 4)\n",
		"  TBD (bit 3)\n",
		"  Supports partial array self refresh\n",
		"  Supports 50 ohm ODT\n",
		"  Supports weak driver\n"
	};

	static const char *decode_row_density_DDR2[] = {
		"512 MiB", "256 MiB", "128 MiB", "16 GiB",
		"8 GiB", "4 GiB", "2 GiB", "1 GiB"
	};

	static const char *decode_row_density_default[] = {
		"512 MiB", "256 MiB", "128 MiB", "64 MiB",
		"32 MiB", "16 MiB", "8 MiB", "4 MiB"
	};

	if (argc < 2)
		return CMD_RET_USAGE;

	/*
	 * Chip is always specified.
	 */
	chip = hextoul(argv[1], NULL);

#if CONFIG_IS_ENABLED(DM_I2C)
	ret = i2c_get_cur_bus_chip(chip, &dev);
	if (!ret)
		ret = dm_i2c_read(dev, 0, data, sizeof(data));
#else
	ret = i2c_read(chip, 0, 1, data, sizeof(data));
#endif
	if (ret) {
		puts ("No SDRAM Serial Presence Detect found.\n");
		return 1;
	}

	cksum = 0;
	for (j = 0; j < 63; j++) {
		cksum += data[j];
	}
	if (cksum != data[63]) {
		printf ("WARNING: Configuration data checksum failure:\n"
			"  is 0x%02x, calculated 0x%02x\n", data[63], cksum);
	}
	printf ("SPD data revision            %d.%d\n",
		(data[62] >> 4) & 0x0F, data[62] & 0x0F);
	printf ("Bytes used                   0x%02X\n", data[0]);
	printf ("Serial memory size           0x%02X\n", 1 << data[1]);

	puts ("Memory type                  ");
	switch (data[2]) {
	case 2:
		type = EDO;
		puts ("EDO\n");
		break;
	case 4:
		type = SDRAM;
		puts ("SDRAM\n");
		break;
	case 7:
		type = DDR;
		puts("DDR\n");
		break;
	case 8:
		type = DDR2;
		puts ("DDR2\n");
		break;
	case 11:
		type = DDR3;
		puts("DDR3\n");
		break;
	case 12:
		type = DDR4;
		puts("DDR4\n");
		break;
	default:
		type = unknown;
		puts ("unknown\n");
		break;
	}

	puts ("Row address bits             ");
	if ((data[3] & 0x00F0) == 0)
		printf ("%d\n", data[3] & 0x0F);
	else
		printf ("%d/%d\n", data[3] & 0x0F, (data[3] >> 4) & 0x0F);

	puts ("Column address bits          ");
	if ((data[4] & 0x00F0) == 0)
		printf ("%d\n", data[4] & 0x0F);
	else
		printf ("%d/%d\n", data[4] & 0x0F, (data[4] >> 4) & 0x0F);

	switch (type) {
	case DDR2:
		printf ("Number of ranks              %d\n",
			(data[5] & 0x07) + 1);
		break;
	default:
		printf ("Module rows                  %d\n", data[5]);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Module data width            %d bits\n", data[6]);
		break;
	default:
		printf ("Module data width            %d bits\n",
			(data[7] << 8) | data[6]);
		break;
	}

	puts ("Interface signal levels      ");
	switch(data[8]) {
		case 0:  puts ("TTL 5.0 V\n");	break;
		case 1:  puts ("LVTTL\n");	break;
		case 2:  puts ("HSTL 1.5 V\n");	break;
		case 3:  puts ("SSTL 3.3 V\n");	break;
		case 4:  puts ("SSTL 2.5 V\n");	break;
		case 5:  puts ("SSTL 1.8 V\n");	break;
		default: puts ("unknown\n");	break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM cycle time             ");
		print_ddr2_tcyc (data[9]);
		break;
	default:
		printf ("SDRAM cycle time             %d.%d ns\n",
			(data[9] >> 4) & 0x0F, data[9] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM access time            0.%d%d ns\n",
			(data[10] >> 4) & 0x0F, data[10] & 0x0F);
		break;
	default:
		printf ("SDRAM access time            %d.%d ns\n",
			(data[10] >> 4) & 0x0F, data[10] & 0x0F);
		break;
	}

	puts ("EDC configuration            ");
	switch (data[11]) {
		case 0:  puts ("None\n");	break;
		case 1:  puts ("Parity\n");	break;
		case 2:  puts ("ECC\n");	break;
		default: puts ("unknown\n");	break;
	}

	if ((data[12] & 0x80) == 0)
		puts ("No self refresh, rate        ");
	else
		puts ("Self refresh, rate           ");

	switch(data[12] & 0x7F) {
		case 0:  puts ("15.625 us\n");	break;
		case 1:  puts ("3.9 us\n");	break;
		case 2:  puts ("7.8 us\n");	break;
		case 3:  puts ("31.3 us\n");	break;
		case 4:  puts ("62.5 us\n");	break;
		case 5:  puts ("125 us\n");	break;
		default: puts ("unknown\n");	break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM width (primary)        %d\n", data[13]);
		break;
	default:
		printf ("SDRAM width (primary)        %d\n", data[13] & 0x7F);
		if ((data[13] & 0x80) != 0) {
			printf ("  (second bank)              %d\n",
				2 * (data[13] & 0x7F));
		}
		break;
	}

	switch (type) {
	case DDR2:
		if (data[14] != 0)
			printf ("EDC width                    %d\n", data[14]);
		break;
	default:
		if (data[14] != 0) {
			printf ("EDC width                    %d\n",
				data[14] & 0x7F);

			if ((data[14] & 0x80) != 0) {
				printf ("  (second bank)              %d\n",
					2 * (data[14] & 0x7F));
			}
		}
		break;
	}

	if (DDR2 != type) {
		printf ("Min clock delay, back-to-back random column addresses "
			"%d\n", data[15]);
	}

	puts ("Burst length(s)             ");
	if (data[16] & 0x80) puts (" Page");
	if (data[16] & 0x08) puts (" 8");
	if (data[16] & 0x04) puts (" 4");
	if (data[16] & 0x02) puts (" 2");
	if (data[16] & 0x01) puts (" 1");
	putc ('\n');
	printf ("Number of banks              %d\n", data[17]);

	switch (type) {
	case DDR2:
		puts ("CAS latency(s)              ");
		decode_bits (data[18], decode_CAS_DDR2, 0);
		putc ('\n');
		break;
	default:
		puts ("CAS latency(s)              ");
		decode_bits (data[18], decode_CAS_default, 0);
		putc ('\n');
		break;
	}

	if (DDR2 != type) {
		puts ("CS latency(s)               ");
		decode_bits (data[19], decode_CS_WE_default, 0);
		putc ('\n');
	}

	if (DDR2 != type) {
		puts ("WE latency(s)               ");
		decode_bits (data[20], decode_CS_WE_default, 0);
		putc ('\n');
	}

	switch (type) {
	case DDR2:
		puts ("Module attributes:\n");
		if (data[21] & 0x80)
			puts ("  TBD (bit 7)\n");
		if (data[21] & 0x40)
			puts ("  Analysis probe installed\n");
		if (data[21] & 0x20)
			puts ("  TBD (bit 5)\n");
		if (data[21] & 0x10)
			puts ("  FET switch external enable\n");
		printf ("  %d PLLs on DIMM\n", (data[21] >> 2) & 0x03);
		if (data[20] & 0x11) {
			printf ("  %d active registers on DIMM\n",
				(data[21] & 0x03) + 1);
		}
		break;
	default:
		puts ("Module attributes:\n");
		if (!data[21])
			puts ("  (none)\n");
		else
			decode_bits (data[21], decode_byte21_default, 0);
		break;
	}

	switch (type) {
	case DDR2:
		decode_bits (data[22], decode_byte22_DDR2, 0);
		break;
	default:
		puts ("Device attributes:\n");
		if (data[22] & 0x80) puts ("  TBD (bit 7)\n");
		if (data[22] & 0x40) puts ("  TBD (bit 6)\n");
		if (data[22] & 0x20) puts ("  Upper Vcc tolerance 5%\n");
		else                 puts ("  Upper Vcc tolerance 10%\n");
		if (data[22] & 0x10) puts ("  Lower Vcc tolerance 5%\n");
		else                 puts ("  Lower Vcc tolerance 10%\n");
		if (data[22] & 0x08) puts ("  Supports write1/read burst\n");
		if (data[22] & 0x04) puts ("  Supports precharge all\n");
		if (data[22] & 0x02) puts ("  Supports auto precharge\n");
		if (data[22] & 0x01) puts ("  Supports early RAS# precharge\n");
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM cycle time (2nd highest CAS latency)        ");
		print_ddr2_tcyc (data[23]);
		break;
	default:
		printf ("SDRAM cycle time (2nd highest CAS latency)        %d."
			"%d ns\n", (data[23] >> 4) & 0x0F, data[23] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM access from clock (2nd highest CAS latency) 0."
			"%d%d ns\n", (data[24] >> 4) & 0x0F, data[24] & 0x0F);
		break;
	default:
		printf ("SDRAM access from clock (2nd highest CAS latency) %d."
			"%d ns\n", (data[24] >> 4) & 0x0F, data[24] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM cycle time (3rd highest CAS latency)        ");
		print_ddr2_tcyc (data[25]);
		break;
	default:
		printf ("SDRAM cycle time (3rd highest CAS latency)        %d."
			"%d ns\n", (data[25] >> 4) & 0x0F, data[25] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM access from clock (3rd highest CAS latency) 0."
			"%d%d ns\n", (data[26] >> 4) & 0x0F, data[26] & 0x0F);
		break;
	default:
		printf ("SDRAM access from clock (3rd highest CAS latency) %d."
			"%d ns\n", (data[26] >> 4) & 0x0F, data[26] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Minimum row precharge        %d.%02d ns\n",
			(data[27] >> 2) & 0x3F, 25 * (data[27] & 0x03));
		break;
	default:
		printf ("Minimum row precharge        %d ns\n", data[27]);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Row active to row active min %d.%02d ns\n",
			(data[28] >> 2) & 0x3F, 25 * (data[28] & 0x03));
		break;
	default:
		printf ("Row active to row active min %d ns\n", data[28]);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("RAS to CAS delay min         %d.%02d ns\n",
			(data[29] >> 2) & 0x3F, 25 * (data[29] & 0x03));
		break;
	default:
		printf ("RAS to CAS delay min         %d ns\n", data[29]);
		break;
	}

	printf ("Minimum RAS pulse width      %d ns\n", data[30]);

	switch (type) {
	case DDR2:
		puts ("Density of each row          ");
		decode_bits (data[31], decode_row_density_DDR2, 1);
		putc ('\n');
		break;
	default:
		puts ("Density of each row          ");
		decode_bits (data[31], decode_row_density_default, 1);
		putc ('\n');
		break;
	}

	switch (type) {
	case DDR2:
		puts ("Command and Address setup    ");
		if (data[32] >= 0xA0) {
			printf ("1.%d%d ns\n",
				((data[32] >> 4) & 0x0F) - 10, data[32] & 0x0F);
		} else {
			printf ("0.%d%d ns\n",
				((data[32] >> 4) & 0x0F), data[32] & 0x0F);
		}
		break;
	default:
		printf ("Command and Address setup    %c%d.%d ns\n",
			(data[32] & 0x80) ? '-' : '+',
			(data[32] >> 4) & 0x07, data[32] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		puts ("Command and Address hold     ");
		if (data[33] >= 0xA0) {
			printf ("1.%d%d ns\n",
				((data[33] >> 4) & 0x0F) - 10, data[33] & 0x0F);
		} else {
			printf ("0.%d%d ns\n",
				((data[33] >> 4) & 0x0F), data[33] & 0x0F);
		}
		break;
	default:
		printf ("Command and Address hold     %c%d.%d ns\n",
			(data[33] & 0x80) ? '-' : '+',
			(data[33] >> 4) & 0x07, data[33] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Data signal input setup      0.%d%d ns\n",
			(data[34] >> 4) & 0x0F, data[34] & 0x0F);
		break;
	default:
		printf ("Data signal input setup      %c%d.%d ns\n",
			(data[34] & 0x80) ? '-' : '+',
			(data[34] >> 4) & 0x07, data[34] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Data signal input hold       0.%d%d ns\n",
			(data[35] >> 4) & 0x0F, data[35] & 0x0F);
		break;
	default:
		printf ("Data signal input hold       %c%d.%d ns\n",
			(data[35] & 0x80) ? '-' : '+',
			(data[35] >> 4) & 0x07, data[35] & 0x0F);
		break;
	}

	puts ("Manufacturer's JEDEC ID      ");
	for (j = 64; j <= 71; j++)
		printf ("%02X ", data[j]);
	putc ('\n');
	printf ("Manufacturing Location       %02X\n", data[72]);
	puts ("Manufacturer's Part Number   ");
	for (j = 73; j <= 90; j++)
		printf ("%02X ", data[j]);
	putc ('\n');
	printf ("Revision Code                %02X %02X\n", data[91], data[92]);
	printf ("Manufacturing Date           %02X %02X\n", data[93], data[94]);
	puts ("Assembly Serial Number       ");
	for (j = 95; j <= 98; j++)
		printf ("%02X ", data[j]);
	putc ('\n');

	if (DDR2 != type) {
		printf ("Speed rating                 PC%d\n",
			data[126] == 0x66 ? 66 : data[126]);
	}
	return 0;
}