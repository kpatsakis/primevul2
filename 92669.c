 void dev_load(struct net *net, const char *name)
 {
 	struct net_device *dev;
	int no_module;
 
 	rcu_read_lock();
 	dev = dev_get_by_name_rcu(net, name);
 	rcu_read_unlock();
 
	no_module = !dev;
	if (no_module && capable(CAP_NET_ADMIN))
		no_module = request_module("netdev-%s", name);
	if (no_module && capable(CAP_SYS_MODULE)) {
		if (!request_module("%s", name))
			pr_err("Loading kernel module for a network device "
"with CAP_SYS_MODULE (deprecated).  Use CAP_NET_ADMIN and alias netdev-%s "
"instead\n", name);
	}
 }
