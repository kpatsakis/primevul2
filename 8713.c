Status UniqueIdxShapeFn(InferenceContext* c) {
  ShapeHandle input = c->input(0);
  const Tensor* axis_t = c->input_tensor(1);
  if (axis_t == nullptr || !c->RankKnown(input)) {
    c->set_output(1, c->Vector(InferenceContext::kUnknownDim));
    return Status::OK();
  }

  if (c->Rank(c->input(1)) != 1) {
    return errors::InvalidArgument("axis expects a 1D vector.");
  }

  int32_t n = axis_t->NumElements();
  if (n == 0) {
    if (c->Rank(input) != 1) {
      return errors::InvalidArgument("x expects a 1D vector.");
    }
    c->set_output(1, input);
    return Status::OK();
  } else if (n == 1) {
    int64_t axis;
    if (axis_t->dtype() == DT_INT32) {
      axis = static_cast<int64_t>(axis_t->flat<int32>()(0));
    } else {
      axis = axis_t->flat<int64_t>()(0);
    }

    int64_t input_rank = c->Rank(input);
    if (axis < -input_rank || axis >= input_rank) {
      return errors::InvalidArgument("axis expects to be in the range [",
                                     -input_rank, ", ", input_rank, ")");
    }
    if (axis < 0) {
      axis += input_rank;
    }
    c->set_output(1, c->Vector(c->Dim(input, axis)));
    return Status::OK();
  }
  return errors::InvalidArgument(
      "axis does not support input tensors larger than 1 elements.");
}