int usbip_recv_xbuff(struct usbip_device *ud, struct urb *urb)
{
	int ret;
	int size;

	if (ud->side == USBIP_STUB) {
		/* the direction of urb must be OUT. */
		if (usb_pipein(urb->pipe))
			return 0;

		size = urb->transfer_buffer_length;
	} else {
		/* the direction of urb must be IN. */
		if (usb_pipeout(urb->pipe))
			return 0;

		size = urb->actual_length;
	}

	/* no need to recv xbuff */
 	if (!(size > 0))
 		return 0;
 
	if (size > urb->transfer_buffer_length) {
		/* should not happen, probably malicious packet */
		if (ud->side == USBIP_STUB) {
			usbip_event_add(ud, SDEV_EVENT_ERROR_TCP);
			return 0;
		} else {
			usbip_event_add(ud, VDEV_EVENT_ERROR_TCP);
			return -EPIPE;
		}
	}

 	ret = usbip_recv(ud->tcp_socket, urb->transfer_buffer, size);
 	if (ret != size) {
 		dev_err(&urb->dev->dev, "recv xbuf, %d\n", ret);
		if (ud->side == USBIP_STUB) {
			usbip_event_add(ud, SDEV_EVENT_ERROR_TCP);
		} else {
			usbip_event_add(ud, VDEV_EVENT_ERROR_TCP);
			return -EPIPE;
		}
	}

	return ret;
}