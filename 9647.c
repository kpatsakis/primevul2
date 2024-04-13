TEST(ComparisonsTest, GreaterQuantized) {
  const float kMin = -1.f;
  const float kMax = 128.f;
  ComparisonOpModel model({TensorType_UINT8, {1, 2, 2, 1}, kMin, kMax},
                          {TensorType_UINT8, {1, 2, 2, 1}, kMin, kMax},
                          TensorType_UINT8, BuiltinOperator_GREATER);
  model.QuantizeAndPopulate<uint8_t>(model.input1(), {1, 9, 7, 3});
  model.QuantizeAndPopulate<uint8_t>(model.input2(), {1, 2, 6, 5});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(false, true, true, false));
}