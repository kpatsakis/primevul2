  ::tensorflow::Status ValidateSplits(
      const std::vector<ConstFlatType>& params_nested_splits,
      SPLITS_TYPE num_params_dense_values) {
    // Validate
    for (int dim = 0; dim < params_nested_splits.size(); ++dim) {
      const auto& splits = params_nested_splits[dim];
      SPLITS_TYPE last_split = (dim == params_nested_splits.size() - 1)
                                   ? num_params_dense_values
                                   : params_nested_splits[dim + 1].size();
      if (splits.size() == 0) {
        return errors::InvalidArgument("Ragged splits may not be empty");
      }
      if (splits(0) < 0) {
        return errors::InvalidArgument("Ragged splits must be non-negative");
      }
      if (splits(splits.size() - 1) > last_split) {
        return errors::InvalidArgument(
            "Ragged splits must not point past values");
      }
      for (int i = 1; i < splits.size(); ++i) {
        if (splits(i - 1) > splits(i)) {
          return errors::InvalidArgument("Ragged splits must be sorted");
        }
      }
    }
    return ::tensorflow::Status::OK();
  }