  void CallWriteValueSlices(
      const Tensor& params_dense_values_in,
      const std::vector<std::pair<SPLITS_TYPE, SPLITS_TYPE>>& value_slices,
      SPLITS_TYPE value_size, Tensor* values_out) const override {
    WriteValueSlices<VALUE_TYPE>(params_dense_values_in, value_slices,
                                 value_size, values_out);
  }