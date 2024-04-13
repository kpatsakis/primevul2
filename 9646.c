TEST(ComparisonsTest, NotEqualBroadcastString) {
  if (SingleOpModel::GetForceUseNnapi()) {
    return;
  }
  ComparisonOpModel model({1, 1, 1, 4}, {1, 1, 1, 1}, TensorType_STRING,
                          BuiltinOperator_NOT_EQUAL);
  model.PopulateTensor<std::string>(model.input1(), {"A", "B", "A", "B"});
  model.PopulateTensor<std::string>(model.input2(), {"A"});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(false, true, false, true));
  EXPECT_THAT(model.GetOutputShape(), ElementsAre(1, 1, 1, 4));
}