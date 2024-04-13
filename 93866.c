static int __net_init sit_init_net(struct net *net)
{
	struct sit_net *sitn = net_generic(net, sit_net_id);
	struct ip_tunnel *t;
	int err;

	sitn->tunnels[0] = sitn->tunnels_wc;
	sitn->tunnels[1] = sitn->tunnels_l;
	sitn->tunnels[2] = sitn->tunnels_r;
	sitn->tunnels[3] = sitn->tunnels_r_l;

	if (!net_has_fallback_tunnels(net))
		return 0;

	sitn->fb_tunnel_dev = alloc_netdev(sizeof(struct ip_tunnel), "sit0",
					   NET_NAME_UNKNOWN,
					   ipip6_tunnel_setup);
	if (!sitn->fb_tunnel_dev) {
		err = -ENOMEM;
		goto err_alloc_dev;
	}
	dev_net_set(sitn->fb_tunnel_dev, net);
	sitn->fb_tunnel_dev->rtnl_link_ops = &sit_link_ops;
	/* FB netdevice is special: we have one, and only one per netns.
	 * Allowing to move it to another netns is clearly unsafe.
	 */
	sitn->fb_tunnel_dev->features |= NETIF_F_NETNS_LOCAL;

	err = register_netdev(sitn->fb_tunnel_dev);
	if (err)
		goto err_reg_dev;

	ipip6_tunnel_clone_6rd(sitn->fb_tunnel_dev, sitn);
	ipip6_fb_tunnel_init(sitn->fb_tunnel_dev);

	t = netdev_priv(sitn->fb_tunnel_dev);

	strcpy(t->parms.name, sitn->fb_tunnel_dev->name);
	return 0;
 
 err_reg_dev:
 	ipip6_dev_free(sitn->fb_tunnel_dev);
	free_netdev(sitn->fb_tunnel_dev);
 err_alloc_dev:
 	return err;
 }
