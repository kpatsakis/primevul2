static void mctp_serial_push_trailer(struct mctp_serial *dev, unsigned char c)
{
	switch (dev->rxpos) {
	case 0:
		dev->rxfcs_rcvd = c << 8;
		dev->rxpos++;
		break;
	case 1:
		dev->rxfcs_rcvd |= c;
		dev->rxpos++;
		break;
	case 2:
		if (c != BYTE_FRAME) {
			dev->rxstate = STATE_ERR;
		} else {
			mctp_serial_rx(dev);
			dev->rxlen = 0;
			dev->rxpos = 0;
			dev->rxstate = STATE_IDLE;
		}
		break;
	}
}