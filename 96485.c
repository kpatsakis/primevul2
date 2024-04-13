static int alloc_apic_access_page(struct kvm *kvm)
{
	struct page *page;
	struct kvm_userspace_memory_region kvm_userspace_mem;
	int r = 0;

	mutex_lock(&kvm->slots_lock);
	if (kvm->arch.apic_access_page)
		goto out;
	kvm_userspace_mem.slot = APIC_ACCESS_PAGE_PRIVATE_MEMSLOT;
	kvm_userspace_mem.flags = 0;
	kvm_userspace_mem.guest_phys_addr = 0xfee00000ULL;
	kvm_userspace_mem.memory_size = PAGE_SIZE;
	r = __kvm_set_memory_region(kvm, &kvm_userspace_mem);
	if (r)
		goto out;

	page = gfn_to_page(kvm, 0xfee00);
	if (is_error_page(page)) {
		r = -EFAULT;
		goto out;
	}

	kvm->arch.apic_access_page = page;
out:
	mutex_unlock(&kvm->slots_lock);
	return r;
}
