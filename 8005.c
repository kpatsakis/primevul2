static void route4_delete_filter_work(struct work_struct *work)
{
	struct route4_filter *f = container_of(to_rcu_work(work),
					       struct route4_filter,
					       rwork);
	rtnl_lock();
	__route4_delete_filter(f);
	rtnl_unlock();
}