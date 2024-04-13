  Tensor AlignTensor(const Tensor& t, const string& name) {
    VLOG(1) << "AlignTensor called for " << name << ", shape "
            << t.shape().DebugString()
            << ". This is unnecessary copying. Consider using shapes with even "
            << "sizes";
    Tensor aligned;
    auto found = pool_.find(name);
    if (found != pool_.end()) {  // found in pool
      CHECK(!found->second.second) << "Tensor " << name << " is in use";
      found->second.second = true;  // mark in use
      aligned = found->second.first;
      CHECK(aligned.shape().IsSameSize(t.shape()));
      CHECK_EQ(aligned.dtype(), t.dtype());
    } else {  // allocate a new temporary tensor
      TF_CHECK_OK(ctx_->allocate_temp(t.dtype(), t.shape(), &aligned));
      pool_.emplace(name, std::make_pair(aligned, true));
    }
    functor::TensorCopyUnaligned<Device, T>()(device_, t.unaligned_flat<T>(),
                                              aligned.flat<T>());
    return aligned;
  }