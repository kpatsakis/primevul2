static int do_i2c_bus_speed(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	int speed, ret=0;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *bus;

	if (i2c_get_cur_bus(&bus))
		return 1;
#endif
	if (argc == 1) {
#if CONFIG_IS_ENABLED(DM_I2C)
		speed = dm_i2c_get_bus_speed(bus);
#else
		speed = i2c_get_bus_speed();
#endif
		/* querying current speed */
		printf("Current bus speed=%d\n", speed);
	} else {
		speed = dectoul(argv[1], NULL);
		printf("Setting bus speed to %d Hz\n", speed);
#if CONFIG_IS_ENABLED(DM_I2C)
		ret = dm_i2c_set_bus_speed(bus, speed);
#else
		ret = i2c_set_bus_speed(speed);
#endif
		if (ret)
			printf("Failure changing bus speed (%d)\n", ret);
	}

	return ret ? CMD_RET_FAILURE : 0;
}