TfLiteStatus GatherStrings(TfLiteContext* context, const TfLiteTensor* input,
                           const TfLiteTensor* positions,
                           TfLiteTensor* output) {
  DynamicBuffer buffer;

  const PositionT* indexes = GetTensorData<PositionT>(positions);
  bool indices_has_only_positive_elements = true;
  const size_t num_indices = positions->bytes / sizeof(PositionT);
  for (size_t i = 0; i < num_indices; i++) {
    if (indexes[i] < 0) {
      indices_has_only_positive_elements = false;
      break;
    }
  }
  TF_LITE_ENSURE(context, indices_has_only_positive_elements);

  const PositionT num_strings = GetStringCount(input);
  const int num_indexes = NumElements(positions);

  for (int i = 0; i < num_indexes; ++i) {
    const PositionT pos = indexes[i];
    TF_LITE_ENSURE(context, pos < num_strings);
    const auto string_ref = GetString(input, pos);
    buffer.AddString(string_ref.str, string_ref.len);
  }
  buffer.WriteToTensor(output, /*new_shape=*/nullptr);
  return kTfLiteOk;
}