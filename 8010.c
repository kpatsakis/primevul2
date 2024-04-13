static void route4_destroy(struct tcf_proto *tp, bool rtnl_held,
			   struct netlink_ext_ack *extack)
{
	struct route4_head *head = rtnl_dereference(tp->root);
	int h1, h2;

	if (head == NULL)
		return;

	for (h1 = 0; h1 <= 256; h1++) {
		struct route4_bucket *b;

		b = rtnl_dereference(head->table[h1]);
		if (b) {
			for (h2 = 0; h2 <= 32; h2++) {
				struct route4_filter *f;

				while ((f = rtnl_dereference(b->ht[h2])) != NULL) {
					struct route4_filter *next;

					next = rtnl_dereference(f->next);
					RCU_INIT_POINTER(b->ht[h2], next);
					tcf_unbind_filter(tp, &f->res);
					if (tcf_exts_get_net(&f->exts))
						route4_queue_work(f);
					else
						__route4_delete_filter(f);
				}
			}
			RCU_INIT_POINTER(head->table[h1], NULL);
			kfree_rcu(b, rcu);
		}
	}
	kfree_rcu(head, rcu);
}