TEST(ComparisonsTest, GreaterQuantizedSmallRange) {
  ComparisonOpModel model({TensorType_UINT8, {1, 2, 2, 1}, 0.0, 1.0},
                          {TensorType_UINT8, {1, 2, 2, 1}, 0.0, 2.0},
                          TensorType_UINT8, BuiltinOperator_GREATER);
  model.QuantizeAndPopulate<uint8_t>(model.input1(), {1.0, 0.5, 0.35, 0.1});
  model.QuantizeAndPopulate<uint8_t>(model.input2(), {1.01, 0.25, 0.3, 0.4});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(false, true, true, false));
}