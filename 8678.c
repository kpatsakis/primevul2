static void mctp_serial_tty_receive_buf(struct tty_struct *tty,
					const unsigned char *c,
					const char *f, int len)
{
	struct mctp_serial *dev = tty->disc_data;
	int i;

	if (!netif_running(dev->netdev))
		return;

	/* we don't (currently) use the flag bytes, just data. */
	for (i = 0; i < len; i++)
		mctp_serial_push(dev, c[i]);
}