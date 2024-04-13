static int powermate_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev (intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct powermate_device *pm;
	struct input_dev *input_dev;
	int pipe, maxp;
 	int error = -ENOMEM;
 
 	interface = intf->cur_altsetting;
	if (interface->desc.bNumEndpoints < 1)
		return -EINVAL;

 	endpoint = &interface->endpoint[0].desc;
 	if (!usb_endpoint_is_int_in(endpoint))
 		return -EIO;

	usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
		0x0a, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0, interface->desc.bInterfaceNumber, NULL, 0,
		USB_CTRL_SET_TIMEOUT);

	pm = kzalloc(sizeof(struct powermate_device), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!pm || !input_dev)
		goto fail1;

	if (powermate_alloc_buffers(udev, pm))
		goto fail2;

	pm->irq = usb_alloc_urb(0, GFP_KERNEL);
	if (!pm->irq)
		goto fail2;

	pm->config = usb_alloc_urb(0, GFP_KERNEL);
	if (!pm->config)
		goto fail3;

	pm->udev = udev;
	pm->intf = intf;
	pm->input = input_dev;

	usb_make_path(udev, pm->phys, sizeof(pm->phys));
	strlcat(pm->phys, "/input0", sizeof(pm->phys));

	spin_lock_init(&pm->lock);

	switch (le16_to_cpu(udev->descriptor.idProduct)) {
	case POWERMATE_PRODUCT_NEW:
		input_dev->name = pm_name_powermate;
		break;
	case POWERMATE_PRODUCT_OLD:
		input_dev->name = pm_name_soundknob;
		break;
	default:
		input_dev->name = pm_name_soundknob;
		printk(KERN_WARNING "powermate: unknown product id %04x\n",
		       le16_to_cpu(udev->descriptor.idProduct));
	}

	input_dev->phys = pm->phys;
	usb_to_input_id(udev, &input_dev->id);
	input_dev->dev.parent = &intf->dev;

	input_set_drvdata(input_dev, pm);

	input_dev->event = powermate_input_event;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL) |
		BIT_MASK(EV_MSC);
	input_dev->keybit[BIT_WORD(BTN_0)] = BIT_MASK(BTN_0);
	input_dev->relbit[BIT_WORD(REL_DIAL)] = BIT_MASK(REL_DIAL);
	input_dev->mscbit[BIT_WORD(MSC_PULSELED)] = BIT_MASK(MSC_PULSELED);

	/* get a handle to the interrupt data pipe */
	pipe = usb_rcvintpipe(udev, endpoint->bEndpointAddress);
	maxp = usb_maxpacket(udev, pipe, usb_pipeout(pipe));

	if (maxp < POWERMATE_PAYLOAD_SIZE_MIN || maxp > POWERMATE_PAYLOAD_SIZE_MAX) {
		printk(KERN_WARNING "powermate: Expected payload of %d--%d bytes, found %d bytes!\n",
			POWERMATE_PAYLOAD_SIZE_MIN, POWERMATE_PAYLOAD_SIZE_MAX, maxp);
		maxp = POWERMATE_PAYLOAD_SIZE_MAX;
	}

	usb_fill_int_urb(pm->irq, udev, pipe, pm->data,
			 maxp, powermate_irq,
			 pm, endpoint->bInterval);
	pm->irq->transfer_dma = pm->data_dma;
	pm->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	/* register our interrupt URB with the USB system */
	if (usb_submit_urb(pm->irq, GFP_KERNEL)) {
		error = -EIO;
		goto fail4;
	}

	error = input_register_device(pm->input);
	if (error)
		goto fail5;


	/* force an update of everything */
	pm->requires_update = UPDATE_PULSE_ASLEEP | UPDATE_PULSE_AWAKE | UPDATE_PULSE_MODE | UPDATE_STATIC_BRIGHTNESS;
	powermate_pulse_led(pm, 0x80, 255, 0, 1, 0); // set default pulse parameters

	usb_set_intfdata(intf, pm);
	return 0;

 fail5:	usb_kill_urb(pm->irq);
 fail4:	usb_free_urb(pm->config);
 fail3:	usb_free_urb(pm->irq);
 fail2:	powermate_free_buffers(udev, pm);
 fail1:	input_free_device(input_dev);
	kfree(pm);
	return error;
}