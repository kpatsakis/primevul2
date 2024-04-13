TEST(ComparisonsTest, LessEqualBroadcastTwoD) {
  ComparisonOpModel model({1, 1, 2, 4}, {1, 1, 1, 4}, TensorType_INT32,
                          BuiltinOperator_LESS_EQUAL);
  model.PopulateTensor<int>(model.input1(), {-1, 9, 7, 3, 2, 4, 2, 8});
  model.PopulateTensor<int>(model.input2(), {7, 1, 2, 4});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(),
              ElementsAre(true, false, false, true, true, false, true, false));
  EXPECT_THAT(model.GetOutputShape(), ElementsAre(1, 1, 2, 4));
}