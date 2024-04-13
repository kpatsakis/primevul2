kvm_irqfd(struct kvm *kvm, struct kvm_irqfd *args)
 {
 	if (args->flags & ~(KVM_IRQFD_FLAG_DEASSIGN | KVM_IRQFD_FLAG_RESAMPLE))
 		return -EINVAL;
	if (args->gsi >= KVM_MAX_IRQ_ROUTES)
		return -EINVAL;
 
 	if (args->flags & KVM_IRQFD_FLAG_DEASSIGN)
 		return kvm_irqfd_deassign(kvm, args);

	return kvm_irqfd_assign(kvm, args);
}
