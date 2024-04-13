static int cxusb_ctrl_msg(struct dvb_usb_device *d,
 			  u8 cmd, u8 *wbuf, int wlen, u8 *rbuf, int rlen)
 {
 	struct cxusb_state *st = d->priv;
	int ret;
 
 	if (1 + wlen > MAX_XFER_SIZE) {
 		warn("i2c wr: len=%d is too big!\n", wlen);
 		return -EOPNOTSUPP;
 	}
 
	if (rlen > MAX_XFER_SIZE) {
		warn("i2c rd: len=%d is too big!\n", rlen);
		return -EOPNOTSUPP;
	}
 
 	mutex_lock(&d->data_mutex);
 	st->data[0] = cmd;
 	memcpy(&st->data[1], wbuf, wlen);
	ret = dvb_usb_generic_rw(d, st->data, 1 + wlen, st->data, rlen, 0);
	if (!ret && rbuf && rlen)
		memcpy(rbuf, st->data, rlen);
 
 	mutex_unlock(&d->data_mutex);
 	return ret;
}
