int bnep_add_connection(struct bnep_connadd_req *req, struct socket *sock)
{
	struct net_device *dev;
	struct bnep_session *s, *ss;
	u8 dst[ETH_ALEN], src[ETH_ALEN];
	int err;
 
 	BT_DBG("");
 
	if (!l2cap_is_socket(sock))
		return -EBADFD;

 	baswap((void *) dst, &l2cap_pi(sock->sk)->chan->dst);
 	baswap((void *) src, &l2cap_pi(sock->sk)->chan->src);
 
	/* session struct allocated as private part of net_device */
	dev = alloc_netdev(sizeof(struct bnep_session),
			   (*req->device) ? req->device : "bnep%d",
			   NET_NAME_UNKNOWN,
			   bnep_net_setup);
	if (!dev)
		return -ENOMEM;

	down_write(&bnep_session_sem);

	ss = __bnep_get_session(dst);
	if (ss && ss->state == BT_CONNECTED) {
		err = -EEXIST;
		goto failed;
	}

	s = netdev_priv(dev);

	/* This is rx header therefore addresses are swapped.
	 * ie. eh.h_dest is our local address. */
	memcpy(s->eh.h_dest,   &src, ETH_ALEN);
	memcpy(s->eh.h_source, &dst, ETH_ALEN);
	memcpy(dev->dev_addr, s->eh.h_dest, ETH_ALEN);

	s->dev   = dev;
	s->sock  = sock;
	s->role  = req->role;
	s->state = BT_CONNECTED;

	s->msg.msg_flags = MSG_NOSIGNAL;

#ifdef CONFIG_BT_BNEP_MC_FILTER
	/* Set default mc filter */
	set_bit(bnep_mc_hash(dev->broadcast), (ulong *) &s->mc_filter);
#endif

#ifdef CONFIG_BT_BNEP_PROTO_FILTER
	/* Set default protocol filter */
	bnep_set_default_proto_filter(s);
#endif

	SET_NETDEV_DEV(dev, bnep_get_device(s));
	SET_NETDEV_DEVTYPE(dev, &bnep_type);

	err = register_netdev(dev);
	if (err)
		goto failed;

	__bnep_link_session(s);

	__module_get(THIS_MODULE);
	s->task = kthread_run(bnep_session, s, "kbnepd %s", dev->name);
	if (IS_ERR(s->task)) {
		/* Session thread start failed, gotta cleanup. */
		module_put(THIS_MODULE);
		unregister_netdev(dev);
		__bnep_unlink_session(s);
		err = PTR_ERR(s->task);
		goto failed;
	}

	up_write(&bnep_session_sem);
	strcpy(req->device, dev->name);
	return 0;

failed:
	up_write(&bnep_session_sem);
	free_netdev(dev);
	return err;
}
