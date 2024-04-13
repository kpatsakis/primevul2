 int usb_cypress_load_firmware(struct usb_device *udev, const struct firmware *fw, int type)
 {
 	struct hexline *hx;
	u8 *buf;
	int ret, pos = 0;
	u16 cpu_cs_register = cypress[type].cpu_cs_register;
 
	buf = kmalloc(sizeof(*hx), GFP_KERNEL);
	if (!buf)
 		return -ENOMEM;
	hx = (struct hexline *)buf;
 
 	/* stop the CPU */
	buf[0] = 1;
	if (usb_cypress_writemem(udev, cpu_cs_register, buf, 1) != 1)
 		err("could not stop the USB controller CPU.");
 
 	while ((ret = dvb_usb_get_hexline(fw, hx, &pos)) > 0) {
		deb_fw("writing to address 0x%04x (buffer: 0x%02x %02x)\n", hx->addr, hx->len, hx->chk);
		ret = usb_cypress_writemem(udev, hx->addr, hx->data, hx->len);

		if (ret != hx->len) {
			err("error while transferring firmware (transferred size: %d, block size: %d)",
				ret, hx->len);
			ret = -EINVAL;
			break;
		}
 	}
 	if (ret < 0) {
 		err("firmware download failed at %d with %d",pos,ret);
		kfree(buf);
 		return ret;
 	}
 
 	if (ret == 0) {
 		/* restart the CPU */
		buf[0] = 0;
		if (usb_cypress_writemem(udev, cpu_cs_register, buf, 1) != 1) {
 			err("could not restart the USB controller CPU.");
 			ret = -EINVAL;
 		}
 	} else
 		ret = -EIO;
 
	kfree(buf);
 
 	return ret;
 }
