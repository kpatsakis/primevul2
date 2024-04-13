Status SetOutputShapeForReshape(InferenceContext* c) {
  ShapeHandle in = c->input(0);
  ShapeHandle out;
  TF_RETURN_IF_ERROR(c->MakeShapeFromShapeTensor(1, &out));

  if (!c->RankKnown(out)) {
    // We have no information about the shape of the output.
    c->set_output(0, out);
    return Status::OK();
  }
  if (c->RankKnown(in)) {
    // We don't know the number of output elements, but we can try to infer
    // the missing dimension.
    bool too_many_unknown = false;
    int32_t out_unknown_idx = -1;

    DimensionHandle known_out_elems = c->NumElements(out);
    if (!c->ValueKnown(known_out_elems)) {
      known_out_elems = c->MakeDim(1);
      for (int32_t i = 0; i < c->Rank(out); ++i) {
        DimensionHandle dim = c->Dim(out, i);
        if (!c->ValueKnown(dim)) {
          if (out_unknown_idx >= 0) {
            too_many_unknown = true;
            break;
          }
          out_unknown_idx = i;
        } else {
          TF_RETURN_IF_ERROR(
              c->Multiply(known_out_elems, dim, &known_out_elems));
        }
      }
    }
    int32_t in_unknown_idx = -1;
    DimensionHandle known_in_elems = c->NumElements(in);
    if (!c->ValueKnown(known_in_elems)) {
      known_in_elems = c->MakeDim(1);
      for (int32_t i = 0; i < c->Rank(in); ++i) {
        DimensionHandle dim = c->Dim(in, i);
        if (!c->ValueKnown(dim)) {
          if (in_unknown_idx >= 0) {
            too_many_unknown = true;
            break;
          }
          in_unknown_idx = i;
        } else {
          TF_RETURN_IF_ERROR(c->Multiply(known_in_elems, dim, &known_in_elems));
        }
      }
    }

    if (!too_many_unknown) {
      if (in_unknown_idx < 0 && out_unknown_idx < 0) {
        // Just check that the dimensions match.
        if (c->Value(known_in_elems) != c->Value(known_out_elems)) {
          return errors::InvalidArgument(
              "Cannot reshape a tensor with ", c->DebugString(known_in_elems),
              " elements to shape ", c->DebugString(out), " (",
              c->DebugString(known_out_elems), " elements)");
        }
      } else if (in_unknown_idx < 0 && out_unknown_idx >= 0 &&
                 c->Value(known_out_elems) > 0) {
        // Input fully known, infer the one missing output dim
        DimensionHandle inferred_dim;
        TF_RETURN_IF_ERROR(c->Divide(known_in_elems, c->Value(known_out_elems),
                                     true /* evenly_divisible */,
                                     &inferred_dim));
        TF_RETURN_IF_ERROR(
            c->ReplaceDim(out, out_unknown_idx, inferred_dim, &out));

      } else if (in_unknown_idx >= 0 && out_unknown_idx < 0 &&
                 c->Value(known_in_elems) != 0) {
        // Output fully known, infer the one missing input dim
        DimensionHandle inferred_dim;
        TF_RETURN_IF_ERROR(c->Divide(known_out_elems, c->Value(known_in_elems),
                                     true /* evenly_divisible */,
                                     &inferred_dim));
        DimensionHandle unknown_in_dim = c->Dim(in, in_unknown_idx);
        TF_RETURN_IF_ERROR(
            c->Merge(unknown_in_dim, inferred_dim, &unknown_in_dim));
      } else if (in_unknown_idx >= 0 && out_unknown_idx >= 0) {
        // Exactly one unknown dimension in both input and output. These 2 are
        // equal iff the known elements are equal.
        if (c->Value(known_in_elems) == c->Value(known_out_elems)) {
          DimensionHandle unknown_in_dim = c->Dim(in, in_unknown_idx);
          TF_RETURN_IF_ERROR(
              c->ReplaceDim(out, out_unknown_idx, unknown_in_dim, &out));
        }
      }
    }
  }
  c->set_output(0, out);
  return Status::OK();
}