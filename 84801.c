NGConstraintSpace NGColumnLayoutAlgorithm::CreateConstraintSpaceForMinMax()
    const {
  NGConstraintSpaceBuilder space_builder(
      ConstraintSpace(), Style().GetWritingMode(), /* is_new_fc */ true);
  space_builder.SetIsAnonymous(true);
  space_builder.SetIsIntermediateLayout(true);

  return space_builder.ToConstraintSpace();
}
