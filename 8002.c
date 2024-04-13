static int route4_delete(struct tcf_proto *tp, void *arg, bool *last,
			 bool rtnl_held, struct netlink_ext_ack *extack)
{
	struct route4_head *head = rtnl_dereference(tp->root);
	struct route4_filter *f = arg;
	struct route4_filter __rcu **fp;
	struct route4_filter *nf;
	struct route4_bucket *b;
	unsigned int h = 0;
	int i, h1;

	if (!head || !f)
		return -EINVAL;

	h = f->handle;
	b = f->bkt;

	fp = &b->ht[from_hash(h >> 16)];
	for (nf = rtnl_dereference(*fp); nf;
	     fp = &nf->next, nf = rtnl_dereference(*fp)) {
		if (nf == f) {
			/* unlink it */
			RCU_INIT_POINTER(*fp, rtnl_dereference(f->next));

			/* Remove any fastmap lookups that might ref filter
			 * notice we unlink'd the filter so we can't get it
			 * back in the fastmap.
			 */
			route4_reset_fastmap(head);

			/* Delete it */
			tcf_unbind_filter(tp, &f->res);
			tcf_exts_get_net(&f->exts);
			tcf_queue_work(&f->rwork, route4_delete_filter_work);

			/* Strip RTNL protected tree */
			for (i = 0; i <= 32; i++) {
				struct route4_filter *rt;

				rt = rtnl_dereference(b->ht[i]);
				if (rt)
					goto out;
			}

			/* OK, session has no flows */
			RCU_INIT_POINTER(head->table[to_hash(h)], NULL);
			kfree_rcu(b, rcu);
			break;
		}
	}

out:
	*last = true;
	for (h1 = 0; h1 <= 256; h1++) {
		if (rcu_access_pointer(head->table[h1])) {
			*last = false;
			break;
		}
	}

	return 0;
}