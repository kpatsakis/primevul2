 static int kvm_vm_ioctl_set_pit(struct kvm *kvm, struct kvm_pit_state *ps)
 {
	int i;
 	mutex_lock(&kvm->arch.vpit->pit_state.lock);
 	memcpy(&kvm->arch.vpit->pit_state, ps, sizeof(struct kvm_pit_state));
	for (i = 0; i < 3; i++)
		kvm_pit_load_count(kvm, i, ps->channels[i].count, 0);
 	mutex_unlock(&kvm->arch.vpit->pit_state.lock);
 	return 0;
 }
