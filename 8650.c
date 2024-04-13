  explicit BoostedTreesMakeStatsSummaryOp(OpKernelConstruction* const context)
      : OpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("max_splits", &max_splits_));
    OP_REQUIRES_OK(context, context->GetAttr("num_buckets", &num_buckets_));
    OP_REQUIRES_OK(context, context->GetAttr("num_features", &num_features_));
  }