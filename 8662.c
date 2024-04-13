  explicit BoostedTreesSparseCalculateBestFeatureSplitOp(
      OpKernelConstruction* const context)
      : OpKernel(context) {
    // TODO(crawles): Using logits_dim_ for multi-class split.
    OP_REQUIRES_OK(context, context->GetAttr("logits_dimension", &logits_dim_));
    // TODO(tanzheny): Using this for equality split.
    OP_REQUIRES_OK(context, context->GetAttr("split_type", &split_type_));
  }