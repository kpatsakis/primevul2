TEST(ComparisonsTest, GreaterInt) {
  ComparisonOpModel model({1, 1, 1, 4}, {1, 1, 1, 4}, TensorType_INT32,
                          BuiltinOperator_GREATER);
  model.PopulateTensor<int>(model.input1(), {-1, 9, 7, 3});
  model.PopulateTensor<int>(model.input2(), {1, 2, 7, 5});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(false, true, false, false));
  EXPECT_THAT(model.GetOutputShape(), ElementsAre(1, 1, 1, 4));
}