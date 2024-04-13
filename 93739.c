void sock_release(struct socket *sock)
static void __sock_release(struct socket *sock, struct inode *inode)
 {
 	if (sock->ops) {
 		struct module *owner = sock->ops->owner;
 
		if (inode)
			inode_lock(inode);
 		sock->ops->release(sock);
		if (inode)
			inode_unlock(inode);
 		sock->ops = NULL;
 		module_put(owner);
 	}

	if (rcu_dereference_protected(sock->wq, 1)->fasync_list)
		pr_err("%s: fasync list not empty!\n", __func__);

	if (!sock->file) {
		iput(SOCK_INODE(sock));
		return;
 	}
 	sock->file = NULL;
 }
