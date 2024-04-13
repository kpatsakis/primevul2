 void ipc_rcu_putref(void *ptr)
 {
	if (!atomic_dec_and_test(&container_of(ptr, struct ipc_rcu_hdr, data)->refcount))
 		return;
 
 	if (container_of(ptr, struct ipc_rcu_hdr, data)->is_vmalloc) {
		call_rcu(&container_of(ptr, struct ipc_rcu_grace, data)->rcu,
				ipc_schedule_free);
	} else {
		kfree_rcu(container_of(ptr, struct ipc_rcu_grace, data), rcu);
	}
}
