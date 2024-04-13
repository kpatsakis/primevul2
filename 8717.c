Status ShapeShapeFn(InferenceContext* c) {
  for (int i = 0; i < c->num_inputs(); ++i) {
    DimensionHandle dim;
    if (c->RankKnown(c->input(i))) {
      dim = c->MakeDim(c->Rank(c->input(i)));
    } else {
      dim = c->UnknownDim();
    }
    c->set_output(i, c->Vector(dim));
  }
  return Status::OK();
}