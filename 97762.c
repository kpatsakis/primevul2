bool kvm_arch_has_assigned_device(struct kvm *kvm)
{
	return atomic_read(&kvm->arch.assigned_device_count);
}
