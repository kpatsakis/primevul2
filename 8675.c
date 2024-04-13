static void mctp_serial_push(struct mctp_serial *dev, unsigned char c)
{
	switch (dev->rxstate) {
	case STATE_IDLE:
		dev->rxstate = STATE_HEADER;
		fallthrough;
	case STATE_HEADER:
		mctp_serial_push_header(dev, c);
		break;

	case STATE_ESCAPE:
		c |= 0x20;
		fallthrough;
	case STATE_DATA:
		if (dev->rxstate != STATE_ESCAPE && c == BYTE_ESC) {
			dev->rxstate = STATE_ESCAPE;
		} else {
			dev->rxfcs = crc_ccitt_byte(dev->rxfcs, c);
			dev->rxbuf[dev->rxpos] = c;
			dev->rxpos++;
			dev->rxstate = STATE_DATA;
			if (dev->rxpos == dev->rxlen) {
				dev->rxpos = 0;
				dev->rxstate = STATE_TRAILER;
			}
		}
		break;

	case STATE_TRAILER:
		mctp_serial_push_trailer(dev, c);
		break;

	case STATE_ERR:
		if (c == BYTE_FRAME)
			dev->rxstate = STATE_IDLE;
		break;

	default:
		netdev_err_once(dev->netdev, "invalid rx state %d\n",
				dev->rxstate);
	}
}