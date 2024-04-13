static void mctp_serial_tty_write_wakeup(struct tty_struct *tty)
{
	struct mctp_serial *dev = tty->disc_data;

	schedule_work(&dev->tx_work);
}