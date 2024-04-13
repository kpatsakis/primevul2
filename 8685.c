static void mctp_serial_push_header(struct mctp_serial *dev, unsigned char c)
{
	switch (dev->rxpos) {
	case 0:
		if (c == BYTE_FRAME)
			dev->rxpos++;
		else
			dev->rxstate = STATE_ERR;
		break;
	case 1:
		if (c == MCTP_SERIAL_VERSION) {
			dev->rxpos++;
			dev->rxfcs = crc_ccitt_byte(0, c);
		} else {
			dev->rxstate = STATE_ERR;
		}
		break;
	case 2:
		if (c > MCTP_SERIAL_FRAME_MTU) {
			dev->rxstate = STATE_ERR;
		} else {
			dev->rxlen = c;
			dev->rxpos = 0;
			dev->rxstate = STATE_DATA;
			dev->rxfcs = crc_ccitt_byte(dev->rxfcs, c);
		}
		break;
	}
}