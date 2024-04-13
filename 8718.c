Status BatchToSpaceShapeHelper(InferenceContext* c, ShapeHandle input_shape,
                               ShapeHandle block_shape_shape,
                               const Tensor* block_shape_t,
                               ShapeHandle crops_shape, const Tensor* crops_t) {
  if (c->Rank(block_shape_shape) != 1) {
    return errors::InvalidArgument("block_shape must have rank 1.");
  }

  const DimensionHandle num_block_dims_handle = c->Dim(block_shape_shape, 0);
  if (!c->ValueKnown(num_block_dims_handle)) {
    return errors::InvalidArgument("block_shape must have known size.");
  }

  const int64_t num_block_dims = c->Value(num_block_dims_handle);

  TF_RETURN_IF_ERROR(
      c->WithRankAtLeast(input_shape, num_block_dims + 1, &input_shape));

  TF_RETURN_IF_ERROR(
      c->Merge(crops_shape, c->Matrix(num_block_dims, 2), &crops_shape));

  DimensionHandle batch_size = c->Dim(input_shape, 0);
  std::vector<int64_t> block_shape_vec;
  if (block_shape_t) {
    block_shape_vec = GetFlatInt64(*block_shape_t);
    for (int64_t dim = 0; dim < num_block_dims; ++dim) {
      const int64_t block_shape_value = block_shape_vec[dim];
      if (block_shape_value < 1) {
        return errors::InvalidArgument("block_shape must be positive");
      }
      if (c->ValueKnown(batch_size)) {
        TF_RETURN_IF_ERROR(c->Divide(batch_size, block_shape_value,
                                     /*evenly_divisible=*/true, &batch_size));
      } else {
        batch_size = c->UnknownDim();
      }
    }
  } else if (num_block_dims > 0) {
    batch_size = c->UnknownDim();
  }

  std::vector<DimensionHandle> output_dims{batch_size};
  output_dims.resize(num_block_dims + 1, c->UnknownDim());

  if (crops_t) {
    const std::vector<int64_t> crops_vec = GetFlatInt64(*crops_t);
    for (int64_t dim = 0; dim < num_block_dims; ++dim) {
      const int64_t crop_start = crops_vec[dim * 2],
                    crop_end = crops_vec[dim * 2 + 1];
      if (crop_start < 0 || crop_end < 0) {
        return errors::InvalidArgument("crops cannot be negative");
      }
      if (block_shape_t) {
        DimensionHandle cropped_size;
        TF_RETURN_IF_ERROR(c->Multiply(c->Dim(input_shape, dim + 1),
                                       block_shape_vec[dim], &cropped_size));
        TF_RETURN_IF_ERROR(
            c->Subtract(cropped_size, crop_start, &cropped_size));
        TF_RETURN_IF_ERROR(
            c->Subtract(cropped_size, crop_end, &output_dims[dim + 1]));
      }
    }
  }

  ShapeHandle remaining_input_shape;
  TF_RETURN_IF_ERROR(
      c->Subshape(input_shape, 1 + num_block_dims, &remaining_input_shape));

  ShapeHandle result;
  TF_RETURN_IF_ERROR(c->Concatenate(c->MakeShape(output_dims),
                                    remaining_input_shape, &result));
  c->set_output(0, result);
  return Status::OK();
}