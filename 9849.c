static int do_i2c_olen(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	uint olen;
	int chip;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	chip = hextoul(argv[1], NULL);
	ret = i2c_get_cur_bus_chip(chip, &dev);
	if (ret)
		return i2c_report_err(ret, I2C_ERR_READ);

	if (argc > 2) {
		olen = hextoul(argv[2], NULL);
		ret = i2c_set_chip_offset_len(dev, olen);
	} else  {
		ret = i2c_get_chip_offset_len(dev);
		if (ret >= 0) {
			printf("%x\n", ret);
			ret = 0;
		}
	}
	if (ret)
		return i2c_report_err(ret, I2C_ERR_READ);

	return 0;
}