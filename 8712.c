Status PadShapeFn(InferenceContext* c) {
  // Paddings is a matrix of [input_rank, 2].
  ShapeHandle paddings;
  TF_RETURN_IF_ERROR(c->WithRank(c->input(1), 2, &paddings));
  DimensionHandle unused;
  TF_RETURN_IF_ERROR(c->WithValue(c->Dim(paddings, 1), 2, &unused));

  // n_dim and input.rank are equivalent.
  ShapeHandle input = c->input(0);
  DimensionHandle n_dim = c->Dim(paddings, 0);
  if (c->ValueKnown(n_dim)) {
    TF_RETURN_IF_ERROR(c->WithRank(input, c->Value(n_dim), &input));
  } else if (c->RankKnown(input)) {
    TF_RETURN_IF_ERROR(c->WithValue(n_dim, c->Rank(input), &n_dim));
  }

  const Tensor* paddings_t = c->input_tensor(1);

  // paddings_t is unknown
  if (paddings_t == nullptr) {
    if (c->ValueKnown(n_dim)) {
      // Make output with n_dim unknown dims.
      c->set_output(0, c->UnknownShapeOfRank(c->Value(n_dim)));
    } else {
      c->set_output(0, c->UnknownShape());
    }
    return Status::OK();
  }

  const int64_t num_dims = paddings_t->shape().dim_size(0);
  TF_RETURN_IF_ERROR(c->WithRank(input, num_dims, &input));
  TF_RETURN_IF_ERROR(c->WithValue(n_dim, num_dims, &n_dim));

  if (paddings_t->dtype() == DT_INT32) {
    return PadKnown<int32>(c, input, paddings_t, num_dims);
  } else {
    return PadKnown<int64_t>(c, input, paddings_t, num_dims);
  }
}