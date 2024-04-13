static int ati_remote2_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(interface);
	struct usb_host_interface *alt = interface->cur_altsetting;
	struct ati_remote2 *ar2;
	int r;

	if (alt->desc.bInterfaceNumber)
		return -ENODEV;

	ar2 = kzalloc(sizeof (struct ati_remote2), GFP_KERNEL);
	if (!ar2)
		return -ENOMEM;
 
 	ar2->udev = udev;
 
	/* Sanity check, first interface must have an endpoint */
	if (alt->desc.bNumEndpoints < 1 || !alt->endpoint) {
		dev_err(&interface->dev,
			"%s(): interface 0 must have an endpoint\n", __func__);
		r = -ENODEV;
		goto fail1;
	}
 	ar2->intf[0] = interface;
 	ar2->ep[0] = &alt->endpoint[0].desc;
 
	/* Sanity check, the device must have two interfaces */
 	ar2->intf[1] = usb_ifnum_to_if(udev, 1);
	if ((udev->actconfig->desc.bNumInterfaces < 2) || !ar2->intf[1]) {
		dev_err(&interface->dev, "%s(): need 2 interfaces, found %d\n",
			__func__, udev->actconfig->desc.bNumInterfaces);
		r = -ENODEV;
		goto fail1;
	}

 	r = usb_driver_claim_interface(&ati_remote2_driver, ar2->intf[1], ar2);
 	if (r)
 		goto fail1;

	/* Sanity check, second interface must have an endpoint */
 	alt = ar2->intf[1]->cur_altsetting;
	if (alt->desc.bNumEndpoints < 1 || !alt->endpoint) {
		dev_err(&interface->dev,
			"%s(): interface 1 must have an endpoint\n", __func__);
		r = -ENODEV;
		goto fail2;
	}
 	ar2->ep[1] = &alt->endpoint[0].desc;
 
 	r = ati_remote2_urb_init(ar2);
 	if (r)
		goto fail3;
 
 	ar2->channel_mask = channel_mask;
 	ar2->mode_mask = mode_mask;
 
 	r = ati_remote2_setup(ar2, ar2->channel_mask);
 	if (r)
		goto fail3;
 
 	usb_make_path(udev, ar2->phys, sizeof(ar2->phys));
 	strlcat(ar2->phys, "/input0", sizeof(ar2->phys));

	strlcat(ar2->name, "ATI Remote Wonder II", sizeof(ar2->name));
 
 	r = sysfs_create_group(&udev->dev.kobj, &ati_remote2_attr_group);
 	if (r)
		goto fail3;
 
 	r = ati_remote2_input_init(ar2);
 	if (r)
		goto fail4;
 
 	usb_set_intfdata(interface, ar2);
 
 	interface->needs_remote_wakeup = 1;
 
 	return 0;
 
 fail4:
 	sysfs_remove_group(&udev->dev.kobj, &ati_remote2_attr_group);
 fail3:
 	ati_remote2_urb_cleanup(ar2);
 fail2:
 	usb_driver_release_interface(&ati_remote2_driver, ar2->intf[1]);
  fail1:
 	kfree(ar2);

	return r;
}