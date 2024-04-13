route4_set_fastmap(struct route4_head *head, u32 id, int iif,
		   struct route4_filter *f)
{
	int h = route4_fastmap_hash(id, iif);

	/* fastmap updates must look atomic to aling id, iff, filter */
	spin_lock_bh(&fastmap_lock);
	head->fastmap[h].id = id;
	head->fastmap[h].iif = iif;
	head->fastmap[h].filter = f;
	spin_unlock_bh(&fastmap_lock);
}