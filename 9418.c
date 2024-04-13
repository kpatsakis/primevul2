  explicit BlockLSTMOp(OpKernelConstruction* ctx) : OpKernel(ctx) {
    if (ctx->HasAttr("forget_bias")) {
      OP_REQUIRES_OK(ctx, ctx->GetAttr("forget_bias", &forget_bias_));
    } else {
      // V2 version does not have "forget_bias" attribute.
      forget_bias_ = 0.0;
    }
    OP_REQUIRES_OK(ctx, ctx->GetAttr("cell_clip", &cell_clip_));
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_peephole", &use_peephole_));
  }