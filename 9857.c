static int do_i2c_show_bus(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	if (argc == 1) {
		/* show all busses */
#if CONFIG_IS_ENABLED(DM_I2C)
		struct udevice *bus;
		struct uclass *uc;
		int ret;

		ret = uclass_get(UCLASS_I2C, &uc);
		if (ret)
			return CMD_RET_FAILURE;
		uclass_foreach_dev(bus, uc)
			show_bus(bus);
#else
		int i;

		for (i = 0; i < CONFIG_SYS_NUM_I2C_BUSES; i++) {
			printf("Bus %d:\t%s", i, I2C_ADAP_NR(i)->name);
#ifndef CONFIG_SYS_I2C_DIRECT_BUS
			int j;

			for (j = 0; j < CONFIG_SYS_I2C_MAX_HOPS; j++) {
				if (i2c_bus[i].next_hop[j].chip == 0)
					break;
				printf("->%s@0x%2x:%d",
				       i2c_bus[i].next_hop[j].mux.name,
				       i2c_bus[i].next_hop[j].chip,
				       i2c_bus[i].next_hop[j].channel);
			}
#endif
			printf("\n");
		}
#endif
	} else {
		int i;

		/* show specific bus */
		i = dectoul(argv[1], NULL);
#if CONFIG_IS_ENABLED(DM_I2C)
		struct udevice *bus;
		int ret;

		ret = uclass_get_device_by_seq(UCLASS_I2C, i, &bus);
		if (ret) {
			printf("Invalid bus %d: err=%d\n", i, ret);
			return CMD_RET_FAILURE;
		}
		show_bus(bus);
#else
		if (i >= CONFIG_SYS_NUM_I2C_BUSES) {
			printf("Invalid bus %d\n", i);
			return -1;
		}
		printf("Bus %d:\t%s", i, I2C_ADAP_NR(i)->name);
#ifndef CONFIG_SYS_I2C_DIRECT_BUS
			int j;
			for (j = 0; j < CONFIG_SYS_I2C_MAX_HOPS; j++) {
				if (i2c_bus[i].next_hop[j].chip == 0)
					break;
				printf("->%s@0x%2x:%d",
				       i2c_bus[i].next_hop[j].mux.name,
				       i2c_bus[i].next_hop[j].chip,
				       i2c_bus[i].next_hop[j].channel);
			}
#endif
		printf("\n");
#endif
	}

	return 0;
}