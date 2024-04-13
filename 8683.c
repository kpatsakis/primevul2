static void __exit mctp_serial_exit(void)
{
	tty_unregister_ldisc(&mctp_ldisc);
}