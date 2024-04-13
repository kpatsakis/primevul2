 void dvb_usbv2_disconnect(struct usb_interface *intf)
 {
 	struct dvb_usb_device *d = usb_get_intfdata(intf);
	const char *devname = kstrdup(dev_name(&d->udev->dev), GFP_KERNEL);
	const char *drvname = d->name;
 
 	dev_dbg(&d->udev->dev, "%s: bInterfaceNumber=%d\n", __func__,
 			intf->cur_altsetting->desc.bInterfaceNumber);

	if (d->props->exit)
		d->props->exit(d);
 
 	dvb_usbv2_exit(d);
 
	pr_info("%s: '%s:%s' successfully deinitialized and disconnected\n",
		KBUILD_MODNAME, drvname, devname);
	kfree(devname);
 }
