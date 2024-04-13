static inline void list_add_leaf_cfs_rq(struct cfs_rq *cfs_rq)
{
	if (!cfs_rq->on_list) {
		struct rq *rq = rq_of(cfs_rq);
		int cpu = cpu_of(rq);
		/*
		 * Ensure we either appear before our parent (if already
		 * enqueued) or force our parent to appear after us when it is
		 * enqueued. The fact that we always enqueue bottom-up
		 * reduces this to two cases and a special case for the root
		 * cfs_rq. Furthermore, it also means that we will always reset
		 * tmp_alone_branch either when the branch is connected
		 * to a tree or when we reach the beg of the tree
		 */
		if (cfs_rq->tg->parent &&
		    cfs_rq->tg->parent->cfs_rq[cpu]->on_list) {
			/*
			 * If parent is already on the list, we add the child
			 * just before. Thanks to circular linked property of
			 * the list, this means to put the child at the tail
			 * of the list that starts by parent.
			 */
			list_add_tail_rcu(&cfs_rq->leaf_cfs_rq_list,
				&(cfs_rq->tg->parent->cfs_rq[cpu]->leaf_cfs_rq_list));
			/*
			 * The branch is now connected to its tree so we can
			 * reset tmp_alone_branch to the beginning of the
			 * list.
			 */
			rq->tmp_alone_branch = &rq->leaf_cfs_rq_list;
		} else if (!cfs_rq->tg->parent) {
			/*
			 * cfs rq without parent should be put
			 * at the tail of the list.
			 */
			list_add_tail_rcu(&cfs_rq->leaf_cfs_rq_list,
				&rq->leaf_cfs_rq_list);
			/*
			 * We have reach the beg of a tree so we can reset
			 * tmp_alone_branch to the beginning of the list.
			 */
			rq->tmp_alone_branch = &rq->leaf_cfs_rq_list;
		} else {
			/*
			 * The parent has not already been added so we want to
			 * make sure that it will be put after us.
			 * tmp_alone_branch points to the beg of the branch
			 * where we will add parent.
			 */
			list_add_rcu(&cfs_rq->leaf_cfs_rq_list,
				rq->tmp_alone_branch);
			/*
			 * update tmp_alone_branch to points to the new beg
			 * of the branch
			 */
			rq->tmp_alone_branch = &cfs_rq->leaf_cfs_rq_list;
		}

		cfs_rq->on_list = 1;
	}
}
