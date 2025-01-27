  explicit BoostedTreesCalculateBestFeatureSplitOp(
      OpKernelConstruction* const context)
      : OpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("logits_dimension", &logits_dim_));
    OP_REQUIRES_OK(context, context->GetAttr("split_type", &split_type_));
  }