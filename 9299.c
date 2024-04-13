  ::tensorflow::Status WriteSplits(
      const std::vector<std::vector<SPLITS_TYPE>>& out_splits,
      OpKernelContext* context) {
    OpOutputList splits_out;
    TF_RETURN_IF_ERROR(
        context->output_list("output_nested_splits", &splits_out));
    for (int i = 0; i < out_splits.size(); ++i) {
      Tensor* splits;
      SPLITS_TYPE num_splits = out_splits[i].size();
      TF_RETURN_IF_ERROR(
          splits_out.allocate(i, TensorShape({num_splits}), &splits));
      auto splits_flat = splits->flat<SPLITS_TYPE>();
      std::copy_n(out_splits[i].data(), out_splits[i].size(),
                  splits_flat.data());
    }
    return ::tensorflow::Status::OK();
  }