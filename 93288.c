 static int omninet_open(struct tty_struct *tty, struct usb_serial_port *port)
 {
 	return usb_serial_generic_open(tty, port);
 }
