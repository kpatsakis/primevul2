TEST(ComparisonsTest, NotEqualBool) {
  ComparisonOpModel model({1, 1, 1, 4}, {1, 1, 1, 4}, TensorType_BOOL,
                          BuiltinOperator_NOT_EQUAL);
  model.PopulateTensor<bool>(model.input1(), {true, false, true, false});
  model.PopulateTensor<bool>(model.input2(), {true, true, false, false});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(false, true, true, false));
  EXPECT_THAT(model.GetOutputShape(), ElementsAre(1, 1, 1, 4));
}