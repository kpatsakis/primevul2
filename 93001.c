 static int cdc_ncm_bind(struct usbnet *dev, struct usb_interface *intf)
 {
 	/* MBIM backwards compatible function? */
 	if (cdc_ncm_select_altsetting(intf) != CDC_NCM_COMM_ALTSETTING_NCM)
 		return -ENODEV;

	/* The NCM data altsetting is fixed, so we hard-coded it.
 	 * Additionally, generic NCM devices are assumed to accept arbitrarily
 	 * placed NDP.
 	 */
	return cdc_ncm_bind_common(dev, intf, CDC_NCM_DATA_ALTSETTING_NCM, 0);
 }
