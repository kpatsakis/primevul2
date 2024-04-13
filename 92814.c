static __init int sctp_init(void)
{
	int i;
	int status = -EINVAL;
	unsigned long goal;
	unsigned long limit;
	int max_share;
	int order;

	sock_skb_cb_check_size(sizeof(struct sctp_ulpevent));

	/* Allocate bind_bucket and chunk caches. */
	status = -ENOBUFS;
	sctp_bucket_cachep = kmem_cache_create("sctp_bind_bucket",
					       sizeof(struct sctp_bind_bucket),
					       0, SLAB_HWCACHE_ALIGN,
					       NULL);
	if (!sctp_bucket_cachep)
		goto out;

	sctp_chunk_cachep = kmem_cache_create("sctp_chunk",
					       sizeof(struct sctp_chunk),
					       0, SLAB_HWCACHE_ALIGN,
					       NULL);
	if (!sctp_chunk_cachep)
		goto err_chunk_cachep;

	status = percpu_counter_init(&sctp_sockets_allocated, 0, GFP_KERNEL);
	if (status)
		goto err_percpu_counter_init;

	/* Implementation specific variables. */

	/* Initialize default stream count setup information. */
	sctp_max_instreams    		= SCTP_DEFAULT_INSTREAMS;
	sctp_max_outstreams   		= SCTP_DEFAULT_OUTSTREAMS;

	/* Initialize handle used for association ids. */
	idr_init(&sctp_assocs_id);

	limit = nr_free_buffer_pages() / 8;
	limit = max(limit, 128UL);
	sysctl_sctp_mem[0] = limit / 4 * 3;
	sysctl_sctp_mem[1] = limit;
	sysctl_sctp_mem[2] = sysctl_sctp_mem[0] * 2;

	/* Set per-socket limits to no more than 1/128 the pressure threshold*/
	limit = (sysctl_sctp_mem[1]) << (PAGE_SHIFT - 7);
	max_share = min(4UL*1024*1024, limit);

	sysctl_sctp_rmem[0] = SK_MEM_QUANTUM; /* give each asoc 1 page min */
	sysctl_sctp_rmem[1] = 1500 * SKB_TRUESIZE(1);
	sysctl_sctp_rmem[2] = max(sysctl_sctp_rmem[1], max_share);

	sysctl_sctp_wmem[0] = SK_MEM_QUANTUM;
	sysctl_sctp_wmem[1] = 16*1024;
	sysctl_sctp_wmem[2] = max(64*1024, max_share);

	/* Size and allocate the association hash table.
	 * The methodology is similar to that of the tcp hash tables.
	 */
	if (totalram_pages >= (128 * 1024))
		goal = totalram_pages >> (22 - PAGE_SHIFT);
	else
		goal = totalram_pages >> (24 - PAGE_SHIFT);

	for (order = 0; (1UL << order) < goal; order++)
		;

	do {
		sctp_assoc_hashsize = (1UL << order) * PAGE_SIZE /
					sizeof(struct sctp_hashbucket);
		if ((sctp_assoc_hashsize > (64 * 1024)) && order > 0)
			continue;
		sctp_assoc_hashtable = (struct sctp_hashbucket *)
			__get_free_pages(GFP_ATOMIC|__GFP_NOWARN, order);
	} while (!sctp_assoc_hashtable && --order > 0);
	if (!sctp_assoc_hashtable) {
		pr_err("Failed association hash alloc\n");
		status = -ENOMEM;
		goto err_ahash_alloc;
	}
	for (i = 0; i < sctp_assoc_hashsize; i++) {
		rwlock_init(&sctp_assoc_hashtable[i].lock);
		INIT_HLIST_HEAD(&sctp_assoc_hashtable[i].chain);
	}

	/* Allocate and initialize the endpoint hash table.  */
	sctp_ep_hashsize = 64;
	sctp_ep_hashtable =
		kmalloc(64 * sizeof(struct sctp_hashbucket), GFP_KERNEL);
	if (!sctp_ep_hashtable) {
		pr_err("Failed endpoint_hash alloc\n");
		status = -ENOMEM;
		goto err_ehash_alloc;
	}
	for (i = 0; i < sctp_ep_hashsize; i++) {
		rwlock_init(&sctp_ep_hashtable[i].lock);
		INIT_HLIST_HEAD(&sctp_ep_hashtable[i].chain);
	}

	/* Allocate and initialize the SCTP port hash table.  */
	do {
		sctp_port_hashsize = (1UL << order) * PAGE_SIZE /
					sizeof(struct sctp_bind_hashbucket);
		if ((sctp_port_hashsize > (64 * 1024)) && order > 0)
			continue;
		sctp_port_hashtable = (struct sctp_bind_hashbucket *)
			__get_free_pages(GFP_ATOMIC|__GFP_NOWARN, order);
	} while (!sctp_port_hashtable && --order > 0);
	if (!sctp_port_hashtable) {
		pr_err("Failed bind hash alloc\n");
		status = -ENOMEM;
		goto err_bhash_alloc;
	}
	for (i = 0; i < sctp_port_hashsize; i++) {
		spin_lock_init(&sctp_port_hashtable[i].lock);
		INIT_HLIST_HEAD(&sctp_port_hashtable[i].chain);
	}

	pr_info("Hash tables configured (established %d bind %d)\n",
		sctp_assoc_hashsize, sctp_port_hashsize);

	sctp_sysctl_register();

	INIT_LIST_HEAD(&sctp_address_families);
 	sctp_v4_pf_init();
 	sctp_v6_pf_init();
 
	status = register_pernet_subsys(&sctp_defaults_ops);
	if (status)
		goto err_register_defaults;
 
	status = sctp_v4_protosw_init();
 	if (status)
 		goto err_protosw_init;
 
 	status = sctp_v6_protosw_init();
 	if (status)
 		goto err_v6_protosw_init;
 
	status = register_pernet_subsys(&sctp_ctrlsock_ops);
 	if (status)
		goto err_register_ctrlsock;
 
 	status = sctp_v4_add_protocol();
 	if (status)
		goto err_add_protocol;

	/* Register SCTP with inet6 layer.  */
	status = sctp_v6_add_protocol();
	if (status)
		goto err_v6_add_protocol;

out:
	return status;
 err_v6_add_protocol:
 	sctp_v4_del_protocol();
 err_add_protocol:
	unregister_pernet_subsys(&sctp_ctrlsock_ops);
err_register_ctrlsock:
 	sctp_v6_protosw_exit();
 err_v6_protosw_init:
 	sctp_v4_protosw_exit();
 err_protosw_init:
	unregister_pernet_subsys(&sctp_defaults_ops);
err_register_defaults:
 	sctp_v4_pf_exit();
 	sctp_v6_pf_exit();
 	sctp_sysctl_unregister();
	free_pages((unsigned long)sctp_port_hashtable,
		   get_order(sctp_port_hashsize *
			     sizeof(struct sctp_bind_hashbucket)));
err_bhash_alloc:
	kfree(sctp_ep_hashtable);
err_ehash_alloc:
	free_pages((unsigned long)sctp_assoc_hashtable,
		   get_order(sctp_assoc_hashsize *
			     sizeof(struct sctp_hashbucket)));
err_ahash_alloc:
	percpu_counter_destroy(&sctp_sockets_allocated);
err_percpu_counter_init:
	kmem_cache_destroy(sctp_chunk_cachep);
err_chunk_cachep:
	kmem_cache_destroy(sctp_bucket_cachep);
	goto out;
}
