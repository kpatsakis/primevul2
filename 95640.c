static bool has_intersects_mems_allowed(struct task_struct *tsk,
					const nodemask_t *mask)
{
	struct task_struct *start = tsk;

	do {
		if (mask) {
			/*
			 * If this is a mempolicy constrained oom, tsk's
			 * cpuset is irrelevant.  Only return true if its
			 * mempolicy intersects current, otherwise it may be
			 * needlessly killed.
			 */
			if (mempolicy_nodemask_intersects(tsk, mask))
				return true;
		} else {
			/*
			 * This is not a mempolicy constrained oom, so only
			 * check the mems of tsk's cpuset.
			 */
			if (cpuset_mems_allowed_intersects(current, tsk))
				return true;
		}
	} while_each_thread(start, tsk);

	return false;
}
