  void FinishTimeStep() {
    for (const auto& p : copy_out_) {
      const Tensor& aligned = p.second;
      Tensor original = p.first;
      // Copy from aligned back to original.
      functor::TensorCopyToUnaligned<Device, T>()(device_, aligned.flat<T>(),
                                                  original.unaligned_flat<T>());
    }
    copy_out_.clear();
    // Mark all entries as not in use.
    for (auto& entry : pool_) {
      entry.second.second = false;
    }
  }