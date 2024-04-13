static void *route4_get(struct tcf_proto *tp, u32 handle)
{
	struct route4_head *head = rtnl_dereference(tp->root);
	struct route4_bucket *b;
	struct route4_filter *f;
	unsigned int h1, h2;

	h1 = to_hash(handle);
	if (h1 > 256)
		return NULL;

	h2 = from_hash(handle >> 16);
	if (h2 > 32)
		return NULL;

	b = rtnl_dereference(head->table[h1]);
	if (b) {
		for (f = rtnl_dereference(b->ht[h2]);
		     f;
		     f = rtnl_dereference(f->next))
			if (f->handle == handle)
				return f;
	}
	return NULL;
}