static int route4_classify(struct sk_buff *skb, const struct tcf_proto *tp,
			   struct tcf_result *res)
{
	struct route4_head *head = rcu_dereference_bh(tp->root);
	struct dst_entry *dst;
	struct route4_bucket *b;
	struct route4_filter *f;
	u32 id, h;
	int iif, dont_cache = 0;

	dst = skb_dst(skb);
	if (!dst)
		goto failure;

	id = dst->tclassid;

	iif = inet_iif(skb);

	h = route4_fastmap_hash(id, iif);

	spin_lock(&fastmap_lock);
	if (id == head->fastmap[h].id &&
	    iif == head->fastmap[h].iif &&
	    (f = head->fastmap[h].filter) != NULL) {
		if (f == ROUTE4_FAILURE) {
			spin_unlock(&fastmap_lock);
			goto failure;
		}

		*res = f->res;
		spin_unlock(&fastmap_lock);
		return 0;
	}
	spin_unlock(&fastmap_lock);

	h = route4_hash_to(id);

restart:
	b = rcu_dereference_bh(head->table[h]);
	if (b) {
		for (f = rcu_dereference_bh(b->ht[route4_hash_from(id)]);
		     f;
		     f = rcu_dereference_bh(f->next))
			if (f->id == id)
				ROUTE4_APPLY_RESULT();

		for (f = rcu_dereference_bh(b->ht[route4_hash_iif(iif)]);
		     f;
		     f = rcu_dereference_bh(f->next))
			if (f->iif == iif)
				ROUTE4_APPLY_RESULT();

		for (f = rcu_dereference_bh(b->ht[route4_hash_wild()]);
		     f;
		     f = rcu_dereference_bh(f->next))
			ROUTE4_APPLY_RESULT();
	}
	if (h < 256) {
		h = 256;
		id &= ~0xFFFF;
		goto restart;
	}

	if (!dont_cache)
		route4_set_fastmap(head, id, iif, ROUTE4_FAILURE);
failure:
	return -1;
}