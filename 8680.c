static int __init mctp_serial_init(void)
{
	return tty_register_ldisc(&mctp_ldisc);
}