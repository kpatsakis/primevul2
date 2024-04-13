TEST(ComparisonsTest, LessBroadcast) {
  ComparisonOpModel model({1, 1, 1, 4}, {1, 1, 1, 1}, TensorType_INT32,
                          BuiltinOperator_LESS);
  model.PopulateTensor<int>(model.input1(), {-1, 9, 7, 3});
  model.PopulateTensor<int>(model.input2(), {7});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(true, false, false, true));
  EXPECT_THAT(model.GetOutputShape(), ElementsAre(1, 1, 1, 4));
}