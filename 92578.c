void ipc_rcu_getref(void *ptr)
int ipc_rcu_getref(void *ptr)
 {
	return atomic_inc_not_zero(&container_of(ptr, struct ipc_rcu_hdr, data)->refcount);
 }
