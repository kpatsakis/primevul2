  explicit LSTMBlockCellGradOp(OpKernelConstruction* ctx) : OpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_peephole", &use_peephole_));
  }