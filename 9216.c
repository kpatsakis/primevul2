TfLiteStatus CalculateShapeForBroadcast(TfLiteContext* context,
                                        const TfLiteTensor* input1,
                                        const TfLiteTensor* input2,
                                        const TfLiteTensor* input3,
                                        TfLiteIntArray** output_shape) {
  const int dims1 = NumDimensions(input1);
  const int dims2 = NumDimensions(input2);
  const int dims3 = NumDimensions(input3);
  const int out_dims = std::max(std::max(dims1, dims2), dims3);
  std::unique_ptr<TfLiteIntArray, void (*)(TfLiteIntArray*)> shape(
      TfLiteIntArrayCreate(out_dims), TfLiteIntArrayFree);
  for (int i = 0; i < out_dims; ++i) {
    const int d1 = i >= dims1 ? 1 : SizeOfDimension(input1, dims1 - i - 1);
    const int d2 = i >= dims2 ? 1 : SizeOfDimension(input2, dims2 - i - 1);
    const int d3 = i >= dims3 ? 1 : SizeOfDimension(input3, dims3 - i - 1);
    const int min_value = std::min(std::min(d1, d2), d3);
    int max_value = std::max(std::max(d1, d2), d3);
    // If one dimention is 0, others must be 0 or 1.
    if (min_value == 0) max_value = 0;
    if (!(d1 == 1 || d1 == max_value) || !(d2 == 1 || d2 == max_value) ||
        !(d3 == 1 || d3 == max_value)) {
      context->ReportError(
          context, "Given shapes, %s, %s and %s, are not broadcastable.",
          GetShapeDebugString(input1->dims).c_str(),
          GetShapeDebugString(input2->dims).c_str(),
          GetShapeDebugString(input3->dims).c_str());
      return kTfLiteError;
    }
    shape->data[out_dims - i - 1] = max_value;
  }
  *output_shape = shape.release();
  return kTfLiteOk;
}