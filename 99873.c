bool may_expand_vm(struct mm_struct *mm, vm_flags_t flags, unsigned long npages)
{
	if (mm->total_vm + npages > rlimit(RLIMIT_AS) >> PAGE_SHIFT)
		return false;

	if (is_data_mapping(flags) &&
	    mm->data_vm + npages > rlimit(RLIMIT_DATA) >> PAGE_SHIFT) {
		/* Workaround for Valgrind */
		if (rlimit(RLIMIT_DATA) == 0 &&
		    mm->data_vm + npages <= rlimit_max(RLIMIT_DATA) >> PAGE_SHIFT)
			return true;

		pr_warn_once("%s (%d): VmData %lu exceed data ulimit %lu. Update limits%s.\n",
			     current->comm, current->pid,
			     (mm->data_vm + npages) << PAGE_SHIFT,
			     rlimit(RLIMIT_DATA),
			     ignore_rlimit_data ? "" : " or use boot option ignore_rlimit_data");

		if (!ignore_rlimit_data)
			return false;
	}

	return true;
}