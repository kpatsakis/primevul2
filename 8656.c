  explicit BoostedTreesCalculateBestFeatureSplitV2(
      OpKernelConstruction* const context)
      : OpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("logits_dimension", &logits_dim_));
    OP_REQUIRES_OK(context, context->GetAttr("num_features", &num_features_));
  }