TEST(ComparisonsTest, EqualString) {
  if (SingleOpModel::GetForceUseNnapi()) {
    return;
  }
  ComparisonOpModel model({1, 1, 1, 4, 1}, {1, 1, 1, 4, 1}, TensorType_STRING,
                          BuiltinOperator_EQUAL);
  model.PopulateTensor<std::string>(model.input1(), {"A", "B", "C", "D"});
  model.PopulateTensor<std::string>(model.input2(), {"A", "C", "B", "D"});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(true, false, false, true));
  EXPECT_THAT(model.GetOutputShape(), ElementsAre(1, 1, 1, 4, 1));
}