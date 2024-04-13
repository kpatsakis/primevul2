static int ati_remote2_pre_reset(struct usb_interface *interface)
{
	struct ati_remote2 *ar2;
	struct usb_host_interface *alt = interface->cur_altsetting;

	if (alt->desc.bInterfaceNumber)
		return 0;

	ar2 = usb_get_intfdata(interface);

	dev_dbg(&ar2->intf[0]->dev, "%s()\n", __func__);

	mutex_lock(&ati_remote2_mutex);

	if (ar2->flags == ATI_REMOTE2_OPENED)
		ati_remote2_kill_urbs(ar2);

	return 0;
}
