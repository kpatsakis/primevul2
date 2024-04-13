static int do_i2c(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *c;

#ifdef CONFIG_NEEDS_MANUAL_RELOC
	i2c_reloc();
#endif

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'i2c' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_i2c_sub[0], ARRAY_SIZE(cmd_i2c_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}