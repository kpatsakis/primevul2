static void __call_console_drivers(unsigned start, unsigned end)
{
	struct console *con;

	for_each_console(con) {
		if (exclusive_console && con != exclusive_console)
			continue;
		if ((con->flags & CON_ENABLED) && con->write &&
				(cpu_online(smp_processor_id()) ||
				(con->flags & CON_ANYTIME)))
			con->write(con, &LOG_BUF(start), end - start);
	}
}
