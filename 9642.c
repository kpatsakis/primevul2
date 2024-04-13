TEST(ComparisonsTest, NotEqualString) {
  if (SingleOpModel::GetForceUseNnapi()) {
    return;
  }
  ComparisonOpModel model({1, 1, 1, 1, 4}, {1, 1, 1, 1, 4}, TensorType_STRING,
                          BuiltinOperator_NOT_EQUAL);
  model.PopulateTensor<std::string>(model.input1(), {"A", "B", "C", "D"});
  model.PopulateTensor<std::string>(model.input2(), {"A", "C", "B", "D"});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(false, true, true, false));
  EXPECT_THAT(model.GetOutputShape(), ElementsAre(1, 1, 1, 1, 4));
}