static int do_i2c_bus_num(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	int		ret = 0;
	int	bus_no;

	if (argc == 1) {
		/* querying current setting */
#if CONFIG_IS_ENABLED(DM_I2C)
		struct udevice *bus;

		if (!i2c_get_cur_bus(&bus))
			bus_no = dev_seq(bus);
		else
			bus_no = -1;
#else
		bus_no = i2c_get_bus_num();
#endif
		printf("Current bus is %d\n", bus_no);
	} else {
		bus_no = dectoul(argv[1], NULL);
#if CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
		if (bus_no >= CONFIG_SYS_NUM_I2C_BUSES) {
			printf("Invalid bus %d\n", bus_no);
			return -1;
		}
#endif
		printf("Setting bus to %d\n", bus_no);
#if CONFIG_IS_ENABLED(DM_I2C)
		ret = cmd_i2c_set_bus_num(bus_no);
#else
		ret = i2c_set_bus_num(bus_no);
#endif
		if (ret)
			printf("Failure changing bus number (%d)\n", ret);
	}

	return ret ? CMD_RET_FAILURE : 0;
}