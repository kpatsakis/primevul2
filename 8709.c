Status ScatterNdTensorShape(InferenceContext* c) {
  ShapeHandle output_shape;
  TF_RETURN_IF_ERROR(c->WithRankAtLeast(c->input(0), 1, &output_shape));
  ShapeHandle indices_shape;
  TF_RETURN_IF_ERROR(c->WithRankAtLeast(c->input(1), 1, &indices_shape));
  ShapeHandle updates_shape;
  TF_RETURN_IF_ERROR(c->WithRankAtLeast(c->input(2), 1, &updates_shape));
  return shape_inference::ScatterNdShapeHelper(c, indices_shape, updates_shape,
                                               output_shape);
}