Status SpaceToBatchShapeHelper(InferenceContext* c, ShapeHandle input_shape,
                               ShapeHandle block_shape_shape,
                               const Tensor* block_shape_t,
                               ShapeHandle paddings_shape,
                               const Tensor* paddings_t) {
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
      c->Merge(paddings_shape, c->Matrix(num_block_dims, 2), &paddings_shape));

  DimensionHandle batch_size = c->Dim(input_shape, 0);
  std::vector<int64_t> block_shape_vec;
  if (block_shape_t && (block_shape_t->NumElements() > 0)) {
    block_shape_vec = GetFlatInt64(*block_shape_t);
    for (int64_t dim = 0; dim < num_block_dims; ++dim) {
      const int64_t block_shape_value = block_shape_vec[dim];
      if (block_shape_value < 1) {
        return errors::InvalidArgument("block_shape must be positive");
      }
      if (c->ValueKnown(batch_size)) {
        TF_RETURN_IF_ERROR(
            c->Multiply(batch_size, block_shape_value, &batch_size));
      } else {
        batch_size = c->UnknownDim();
      }
    }
  } else if (num_block_dims > 0) {
    batch_size = c->UnknownDim();
  }

  std::vector<DimensionHandle> output_dims{batch_size};
  output_dims.resize(num_block_dims + 1, c->UnknownDim());

  if (paddings_t && (paddings_t->NumElements() > 0)) {
    const std::vector<int64_t> paddings_vec = GetFlatInt64(*paddings_t);
    for (int64_t dim = 0; dim < num_block_dims; ++dim) {
      const int64_t pad_start = paddings_vec[dim * 2],
                    pad_end = paddings_vec[dim * 2 + 1];
      if (pad_start < 0 || pad_end < 0) {
        return errors::InvalidArgument("paddings cannot be negative");
      }
      if (block_shape_t) {
        DimensionHandle padded_size;
        TF_RETURN_IF_ERROR(
            c->Add(c->Dim(input_shape, dim + 1), pad_start, &padded_size));
        TF_RETURN_IF_ERROR(c->Add(padded_size, pad_end, &padded_size));
        TF_RETURN_IF_ERROR(c->Divide(padded_size, block_shape_vec[dim],
                                     /*evenly_divisible=*/true,
                                     &output_dims[dim + 1]));
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