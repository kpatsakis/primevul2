  ::tensorflow::Status WriteValues(
      const Tensor& params_dense_values_in,
      const std::vector<std::pair<SPLITS_TYPE, SPLITS_TYPE>>& value_slices,
      int values_index, SPLITS_TYPE num_values,
      OpKernelContext* context) const {
    Tensor* values_out = nullptr;
    TensorShape values_shape = params_dense_values_in.shape();
    values_shape.set_dim(0, num_values);
    TF_RETURN_IF_ERROR(
        context->allocate_output(values_index, values_shape, &values_out));
    const SPLITS_TYPE num_elements = params_dense_values_in.NumElements();
    const SPLITS_TYPE value_size =
        num_elements == 0 ? 0
                          : (num_elements / params_dense_values_in.dim_size(0));
    CallWriteValueSlices(params_dense_values_in, value_slices, value_size,
                         values_out);
    return ::tensorflow::Status::OK();
  }