TEST(QuantizedComparisonsTest, EqualInt8Quantized) {
  const float kMin = -127.f;
  const float kMax = 127.f;
  ComparisonOpModel model({TensorType_INT8, {1, 2, 2, 1}, kMin, kMax},
                          {TensorType_INT8, {1, 2, 2, 1}, kMin, kMax},
                          TensorType_INT8, BuiltinOperator_EQUAL);
  model.QuantizeAndPopulate<int8_t>(model.input1(), {1, -9, 7, 3});
  model.QuantizeAndPopulate<int8_t>(model.input2(), {-1, 2, 7, 5});
  model.Invoke();

  EXPECT_THAT(model.GetOutput(), ElementsAre(false, false, true, false));
}