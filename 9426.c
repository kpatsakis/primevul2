  explicit SliceHelper(OpKernelContext* ctx)
      : ctx_(ctx), device_(ctx_->eigen_device<Device>()) {}