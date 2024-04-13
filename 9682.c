TEST(ComparisonsTest, QuantizedInt8LessWithBroadcast) {
  const float kMin = -127.f;
  const float kMax = 127.f;
  std::vector<std::vector<int>> test_shapes = {
      {6}, {2, 3}, {2, 1, 3}, {1, 3, 1, 2}};
  for (int i = 0; i < test_shapes.size(); ++i) {
    ComparisonOpModel model({TensorType_INT8, test_shapes[i], kMin, kMax},
                            {TensorType_INT8, {}, kMin, kMax}, TensorType_INT8,
                            BuiltinOperator_LESS);
    model.QuantizeAndPopulate<int8_t>(model.input1(), {20, -2, -71, 8, 11, 20});
    model.QuantizeAndPopulate<int8_t>(model.input2(), {8});
    model.Invoke();
    EXPECT_THAT(model.GetOutput(),
                ElementsAre(false, true, true, false, false, false))
        << "With shape number " << i;
  }
}