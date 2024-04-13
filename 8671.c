static void mctp_serial_tx_work(struct work_struct *work)
{
	struct mctp_serial *dev = container_of(work, struct mctp_serial,
					       tx_work);
	unsigned char c, buf[3];
	unsigned long flags;
	int len, txlen;

	spin_lock_irqsave(&dev->lock, flags);

	/* txstate represents the next thing to send */
	switch (dev->txstate) {
	case STATE_START:
		dev->txpos = 0;
		fallthrough;
	case STATE_HEADER:
		buf[0] = BYTE_FRAME;
		buf[1] = MCTP_SERIAL_VERSION;
		buf[2] = dev->txlen;

		if (!dev->txpos)
			dev->txfcs = crc_ccitt(0, buf + 1, 2);

		txlen = write_chunk(dev, buf + dev->txpos, 3 - dev->txpos);
		if (txlen <= 0) {
			dev->txstate = STATE_ERR;
		} else {
			dev->txpos += txlen;
			if (dev->txpos == 3) {
				dev->txstate = STATE_DATA;
				dev->txpos = 0;
			}
		}
		break;

	case STATE_ESCAPE:
		buf[0] = dev->txbuf[dev->txpos] & ~0x20;
		txlen = write_chunk(dev, buf, 1);
		if (txlen <= 0) {
			dev->txstate = STATE_ERR;
		} else {
			dev->txpos += txlen;
			if (dev->txpos == dev->txlen) {
				dev->txstate = STATE_TRAILER;
				dev->txpos = 0;
			}
		}

		break;

	case STATE_DATA:
		len = next_chunk_len(dev);
		if (len) {
			c = dev->txbuf[dev->txpos];
			if (len == 1 && needs_escape(c)) {
				buf[0] = BYTE_ESC;
				buf[1] = c & ~0x20;
				dev->txfcs = crc_ccitt_byte(dev->txfcs, c);
				txlen = write_chunk(dev, buf, 2);
				if (txlen == 2)
					dev->txpos++;
				else if (txlen == 1)
					dev->txstate = STATE_ESCAPE;
				else
					dev->txstate = STATE_ERR;
			} else {
				txlen = write_chunk(dev,
						    dev->txbuf + dev->txpos,
						    len);
				if (txlen <= 0) {
					dev->txstate = STATE_ERR;
				} else {
					dev->txfcs = crc_ccitt(dev->txfcs,
							       dev->txbuf +
							       dev->txpos,
							       txlen);
					dev->txpos += txlen;
				}
			}
			if (dev->txstate == STATE_DATA &&
			    dev->txpos == dev->txlen) {
				dev->txstate = STATE_TRAILER;
				dev->txpos = 0;
			}
			break;
		}
		dev->txstate = STATE_TRAILER;
		dev->txpos = 0;
		fallthrough;

	case STATE_TRAILER:
		buf[0] = dev->txfcs >> 8;
		buf[1] = dev->txfcs & 0xff;
		buf[2] = BYTE_FRAME;
		txlen = write_chunk(dev, buf + dev->txpos, 3 - dev->txpos);
		if (txlen <= 0) {
			dev->txstate = STATE_ERR;
		} else {
			dev->txpos += txlen;
			if (dev->txpos == 3) {
				dev->txstate = STATE_DONE;
				dev->txpos = 0;
			}
		}
		break;
	default:
		netdev_err_once(dev->netdev, "invalid tx state %d\n",
				dev->txstate);
	}

	if (dev->txstate == STATE_DONE) {
		dev->netdev->stats.tx_packets++;
		dev->netdev->stats.tx_bytes += dev->txlen;
		dev->txlen = 0;
		dev->txpos = 0;
		clear_bit(TTY_DO_WRITE_WAKEUP, &dev->tty->flags);
		dev->txstate = STATE_IDLE;
		spin_unlock_irqrestore(&dev->lock, flags);

		netif_wake_queue(dev->netdev);
	} else {
		spin_unlock_irqrestore(&dev->lock, flags);
	}
}