TEST(ComparisonsTest, QuantizedEqualWithBroadcast) {
  const float kMin = -1.f;
  const float kMax = 128.f;
  std::vector<std::vector<int>> test_shapes = {
      {6}, {2, 3}, {2, 1, 3}, {1, 3, 1, 2}};
  for (int i = 0; i < test_shapes.size(); ++i) {
    ComparisonOpModel model({TensorType_UINT8, test_shapes[i], kMin, kMax},
                            {TensorType_UINT8, {}, kMin, kMax},
                            TensorType_UINT8, BuiltinOperator_EQUAL);
    model.QuantizeAndPopulate<uint8_t>(model.input1(), {20, 2, 7, 8, 11, 20});
    model.QuantizeAndPopulate<uint8_t>(model.input2(), {2});
    model.Invoke();
    EXPECT_THAT(model.GetOutput(),
                ElementsAre(false, true, false, false, false, false))
        << "With shape number " << i;
  }
}