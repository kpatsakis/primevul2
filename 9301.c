  ::tensorflow::Status ValidateIndices(const Tensor& indices_in,
                                       SPLITS_TYPE num_params) {
    const auto& indices = indices_in.flat<INDEX_TYPE>();
    for (SPLITS_TYPE i = 0; i < indices.size(); ++i) {
      SPLITS_TYPE index = indices(i);
      if (index < 0 || index >= num_params) {
        return errors::InvalidArgument(
            "indices", SliceDebugString(indices_in.shape(), i), " = ", index,
            " is not in [0, ", num_params, ")");
      }
    }
    return ::tensorflow::Status::OK();
  }