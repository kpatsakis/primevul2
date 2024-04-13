static void nr_set_lockdep_one(struct net_device *dev,
			       struct netdev_queue *txq,
			       void *_unused)
{
	lockdep_set_class(&txq->_xmit_lock, &nr_netdev_xmit_lock_key);
}
