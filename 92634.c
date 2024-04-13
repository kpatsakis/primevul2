static ssize_t wdm_read
(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
	int rv, cntr;
	int i = 0;
	struct wdm_device *desc = file->private_data;


	rv = mutex_lock_interruptible(&desc->rlock); /*concurrent reads */
	if (rv < 0)
		return -ERESTARTSYS;

	cntr = ACCESS_ONCE(desc->length);
	if (cntr == 0) {
		desc->read = 0;
retry:
		if (test_bit(WDM_DISCONNECTING, &desc->flags)) {
 			rv = -ENODEV;
 			goto err;
 		}
		if (test_bit(WDM_OVERFLOW, &desc->flags)) {
			clear_bit(WDM_OVERFLOW, &desc->flags);
			rv = -ENOBUFS;
			goto err;
		}
 		i++;
 		if (file->f_flags & O_NONBLOCK) {
 			if (!test_bit(WDM_READ, &desc->flags)) {
				rv = cntr ? cntr : -EAGAIN;
				goto err;
			}
			rv = 0;
		} else {
			rv = wait_event_interruptible(desc->wait,
				test_bit(WDM_READ, &desc->flags));
		}

		/* may have happened while we slept */
		if (test_bit(WDM_DISCONNECTING, &desc->flags)) {
			rv = -ENODEV;
			goto err;
		}
		if (test_bit(WDM_RESETTING, &desc->flags)) {
			rv = -EIO;
			goto err;
		}
		usb_mark_last_busy(interface_to_usbdev(desc->intf));
		if (rv < 0) {
			rv = -ERESTARTSYS;
			goto err;
		}

		spin_lock_irq(&desc->iuspin);

		if (desc->rerr) { /* read completed, error happened */
			desc->rerr = 0;
			spin_unlock_irq(&desc->iuspin);
			rv = -EIO;
			goto err;
		}
		/*
		 * recheck whether we've lost the race
		 * against the completion handler
		 */
		if (!test_bit(WDM_READ, &desc->flags)) { /* lost race */
 			spin_unlock_irq(&desc->iuspin);
 			goto retry;
 		}

 		if (!desc->reslength) { /* zero length read */
 			dev_dbg(&desc->intf->dev, "%s: zero length - clearing WDM_READ\n", __func__);
 			clear_bit(WDM_READ, &desc->flags);
			spin_unlock_irq(&desc->iuspin);
			goto retry;
		}
		cntr = desc->length;
		spin_unlock_irq(&desc->iuspin);
	}

	if (cntr > count)
		cntr = count;
	rv = copy_to_user(buffer, desc->ubuf, cntr);
	if (rv > 0) {
		rv = -EFAULT;
		goto err;
	}

	spin_lock_irq(&desc->iuspin);

	for (i = 0; i < desc->length - cntr; i++)
		desc->ubuf[i] = desc->ubuf[i + cntr];

	desc->length -= cntr;
	/* in case we had outstanding data */
	if (!desc->length)
		clear_bit(WDM_READ, &desc->flags);

	spin_unlock_irq(&desc->iuspin);

	rv = cntr;

err:
	mutex_unlock(&desc->rlock);
	return rv;
}