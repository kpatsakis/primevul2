static int write_chunk(struct mctp_serial *dev, unsigned char *buf, int len)
{
	return dev->tty->ops->write(dev->tty, buf, len);
}