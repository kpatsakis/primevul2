TEST(ComparisonsTest, LessFloat) {
  ComparisonOpModel model({1, 1, 1, 4}, {1, 1, 1, 4}, TensorType_FLOAT32,
                          BuiltinOperator_LESS);
  model.PopulateTensor<float>(model.input1(), {0.1, 0.9, 0.7, 0.3});
  model.PopulateTensor<float>(model.input2(), {0.1, 0.2, 0.6, 0.5});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(false, false, false, true));
  EXPECT_THAT(model.GetOutputShape(), ElementsAre(1, 1, 1, 4));
}