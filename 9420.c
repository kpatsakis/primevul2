  void Compute(OpKernelContext* ctx) override {
    const Tensor* seq_len_max_tensor = nullptr;
    OP_REQUIRES_OK(ctx, ctx->input("seq_len_max", &seq_len_max_tensor));

    const Tensor* x;
    OP_REQUIRES_OK(ctx, ctx->input("x", &x));
    OP_REQUIRES(ctx, x->dims() == 3, errors::InvalidArgument("x must be 3D"));
    const int64_t timelen = x->dim_size(0);
    const int64_t batch_size = x->dim_size(1);
    const int64_t input_size = x->dim_size(2);

    const Tensor* cs_prev_tensor = nullptr;
    OP_REQUIRES_OK(ctx, ctx->input("cs_prev", &cs_prev_tensor));
    OP_REQUIRES(ctx, cs_prev_tensor->dims() == 2,
                errors::InvalidArgument("cs_prev must be 2D"));
    OP_REQUIRES(ctx, cs_prev_tensor->dim_size(0) == batch_size,
                errors::InvalidArgument("cs_prev.dims(0) != batch_size: ",
                                        cs_prev_tensor->dim_size(0), " vs. ",
                                        batch_size));
    const int64_t cell_size = cs_prev_tensor->dim_size(1);

    if (batch_size * input_size % 2 == 1) {
      LOG(WARNING) << "BlockLSTMOp is inefficient when both batch_size and "
                   << "input_size are odd. You are using: batch_size="
                   << batch_size << ", input_size=" << input_size;
    }
    if (batch_size * cell_size % 2 == 1) {
      LOG(WARNING) << "BlockLSTMOp is inefficient when both batch_size and "
                   << "cell_size are odd. You are using: batch_size="
                   << batch_size << ", cell_size=" << cell_size;
    }

    const Tensor* h_prev_tensor = nullptr;
    OP_REQUIRES_OK(ctx, ctx->input("h_prev", &h_prev_tensor));
    OP_REQUIRES(ctx, h_prev_tensor->dims() == 2,
                errors::InvalidArgument("h_prev must be 2D"));
    OP_REQUIRES(ctx, h_prev_tensor->dim_size(0) == batch_size,
                errors::InvalidArgument("h_prev.dims(0) != batch_size: ",
                                        h_prev_tensor->dim_size(0), " vs. ",
                                        batch_size));
    OP_REQUIRES(ctx, h_prev_tensor->dim_size(1) == cell_size,
                errors::InvalidArgument(
                    "h_prev.dims(1) != cell_size: ", h_prev_tensor->dim_size(1),
                    " vs. ", cell_size));

    const Tensor* w_tensor = nullptr;
    OP_REQUIRES_OK(ctx, ctx->input("w", &w_tensor));
    OP_REQUIRES(ctx, w_tensor->dims() == 2,
                errors::InvalidArgument("w must be 2D"));
    OP_REQUIRES(ctx, w_tensor->dim_size(0) == input_size + cell_size,
                errors::InvalidArgument(
                    "w.dim_size(0) != input_size + cell_size: ",
                    w_tensor->dim_size(0), " vs. ", input_size + cell_size));
    OP_REQUIRES(ctx, w_tensor->dim_size(1) == cell_size * 4,
                errors::InvalidArgument(
                    "w.dim_size(1) != cell_size * 4: ", w_tensor->dim_size(1),
                    " vs. ", cell_size * 4));

    const Tensor* wci_tensor = nullptr;
    OP_REQUIRES_OK(ctx, ctx->input("wci", &wci_tensor));
    OP_REQUIRES(ctx, wci_tensor->dims() == 1,
                errors::InvalidArgument("wci must be 1D"));
    OP_REQUIRES(ctx, wci_tensor->dim_size(0) == cell_size,
                errors::InvalidArgument(
                    "wci.dim_size(0) != cell_size: ", wci_tensor->dim_size(0),
                    " vs. ", cell_size));

    const Tensor* wcf_tensor = nullptr;
    OP_REQUIRES_OK(ctx, ctx->input("wcf", &wcf_tensor));
    OP_REQUIRES(ctx, wcf_tensor->dims() == 1,
                errors::InvalidArgument("wcf must be 1D"));
    OP_REQUIRES(ctx, wcf_tensor->dim_size(0) == cell_size,
                errors::InvalidArgument(
                    "wcf.dim_size(0) != cell_size: ", wcf_tensor->dim_size(0),
                    " vs. ", cell_size));

    const Tensor* wco_tensor = nullptr;
    OP_REQUIRES_OK(ctx, ctx->input("wco", &wco_tensor));
    OP_REQUIRES(ctx, wco_tensor->dims() == 1,
                errors::InvalidArgument("wco must be 1D"));
    OP_REQUIRES(ctx, wco_tensor->dim_size(0) == cell_size,
                errors::InvalidArgument(
                    "wco.dim_size(0) != cell_size: ", wco_tensor->dim_size(0),
                    " vs. ", cell_size));

    const Tensor* b_tensor = nullptr;
    OP_REQUIRES_OK(ctx, ctx->input("b", &b_tensor));
    OP_REQUIRES(ctx, b_tensor->dims() == 1,
                errors::InvalidArgument("b must be 1D"));
    OP_REQUIRES(ctx, b_tensor->dim_size(0) == cell_size * 4,
                errors::InvalidArgument(
                    "b.dim_size(0) != cell_size * 4: ", b_tensor->dim_size(0),
                    " vs. ", cell_size * 4));

    TensorShape batch_cell_shape({timelen, batch_size, cell_size});
    Tensor* i_out;
    OP_REQUIRES_OK(ctx, ctx->allocate_output("i", batch_cell_shape, &i_out));

    Tensor* cs_out;
    OP_REQUIRES_OK(ctx, ctx->allocate_output("cs", batch_cell_shape, &cs_out));

    Tensor* f_out;
    OP_REQUIRES_OK(ctx, ctx->allocate_output("f", batch_cell_shape, &f_out));

    Tensor* o_out;
    OP_REQUIRES_OK(ctx, ctx->allocate_output("o", batch_cell_shape, &o_out));

    Tensor* ci_out;
    OP_REQUIRES_OK(ctx, ctx->allocate_output("ci", batch_cell_shape, &ci_out));

    Tensor* co_out;
    OP_REQUIRES_OK(ctx, ctx->allocate_output("co", batch_cell_shape, &co_out));

    Tensor* h_out;
    OP_REQUIRES_OK(ctx, ctx->allocate_output("h", batch_cell_shape, &h_out));

    Tensor xh_tensor;
    OP_REQUIRES_OK(ctx, ctx->allocate_temp(
                            DataTypeToEnum<T>::v(),
                            TensorShape({batch_size, input_size + cell_size}),
                            &xh_tensor));

    Tensor gates_tensor;
    OP_REQUIRES_OK(ctx,
                   ctx->allocate_temp(DataTypeToEnum<T>::v(),
                                      TensorShape({batch_size, cell_size * 4}),
                                      &gates_tensor));

    const Device& device = ctx->eigen_device<Device>();

    const int64_t seq_len_max = seq_len_max_tensor->scalar<int64_t>()();
    SliceHelper<Device, T> slicer(ctx);
    for (int64_t t = 0; t < seq_len_max; ++t) {
      const Tensor x_tensor = slicer.InputSlice(*x, t, "x");
      const Tensor& cs_prev_tensor2 =
          t == 0 ? *cs_prev_tensor
                 : slicer.OutputSlice(cs_out, t - 1, "cs_prev");
      const Tensor& h_prev_tensor2 =
          t == 0 ? *h_prev_tensor : slicer.OutputSlice(h_out, t - 1, "h_prev");

      Tensor i_tensor = slicer.OutputSlice(i_out, t, "i_out");
      Tensor cs_tensor = slicer.OutputSlice(cs_out, t, "cs_out");
      Tensor f_tensor = slicer.OutputSlice(f_out, t, "f_out");
      Tensor o_tensor = slicer.OutputSlice(o_out, t, "o_out");
      Tensor ci_tensor = slicer.OutputSlice(ci_out, t, "ci_out");
      Tensor co_tensor = slicer.OutputSlice(co_out, t, "co_out");
      Tensor h_tensor = slicer.OutputSlice(h_out, t, "h_out");

      functor::LSTMBlockCellFprop<Device, T, USE_CUBLAS, gate_layout>(
          batch_size, input_size, cell_size)(
          ctx, device, forget_bias_, cell_clip_, use_peephole_,
          x_tensor.matrix<T>(), cs_prev_tensor2.matrix<T>(),
          h_prev_tensor2.matrix<T>(), w_tensor->matrix<T>(),
          wci_tensor->vec<T>(), wcf_tensor->vec<T>(), wco_tensor->vec<T>(),
          b_tensor->vec<T>(), xh_tensor.matrix<T>(), i_tensor.matrix<T>(),
          cs_tensor.matrix<T>(), f_tensor.matrix<T>(), o_tensor.matrix<T>(),
          ci_tensor.matrix<T>(), co_tensor.matrix<T>(),
          gates_tensor.matrix<T>(), h_tensor.matrix<T>());
      slicer.FinishTimeStep();
    }

    if (seq_len_max < timelen) {
      Tensor cs_tensor = cs_out->Slice(seq_len_max, timelen);
      Tensor h_tensor = h_out->Slice(seq_len_max, timelen);

      functor::TensorUnalignedZero<Device, T>()(device,
                                                cs_tensor.unaligned_flat<T>());
      functor::TensorUnalignedZero<Device, T>()(device,
                                                h_tensor.unaligned_flat<T>());
    }
  }