static int mctp_serial_open(struct tty_struct *tty)
{
	struct mctp_serial *dev;
	struct net_device *ndev;
	char name[32];
	int idx, rc;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!tty->ops->write)
		return -EOPNOTSUPP;

	idx = ida_alloc(&mctp_serial_ida, GFP_KERNEL);
	if (idx < 0)
		return idx;

	snprintf(name, sizeof(name), "mctpserial%d", idx);
	ndev = alloc_netdev(sizeof(*dev), name, NET_NAME_ENUM,
			    mctp_serial_setup);
	if (!ndev) {
		rc = -ENOMEM;
		goto free_ida;
	}

	dev = netdev_priv(ndev);
	dev->idx = idx;
	dev->tty = tty;
	dev->netdev = ndev;
	dev->txstate = STATE_IDLE;
	dev->rxstate = STATE_IDLE;
	spin_lock_init(&dev->lock);
	INIT_WORK(&dev->tx_work, mctp_serial_tx_work);

	rc = register_netdev(ndev);
	if (rc)
		goto free_netdev;

	tty->receive_room = 64 * 1024;
	tty->disc_data = dev;

	return 0;

free_netdev:
	free_netdev(ndev);

free_ida:
	ida_free(&mctp_serial_ida, idx);
	return rc;
}