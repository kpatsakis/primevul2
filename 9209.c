TfLiteStatus CalculateShapeForBroadcast(TfLiteContext* context,
                                        const TfLiteTensor* input1,
                                        const TfLiteTensor* input2,
                                        TfLiteIntArray** output_shape) {
  const int dims1 = NumDimensions(input1);
  const int dims2 = NumDimensions(input2);
  const int out_dims = std::max(dims1, dims2);

  std::unique_ptr<TfLiteIntArray, void (*)(TfLiteIntArray*)> shape(
      TfLiteIntArrayCreate(out_dims), TfLiteIntArrayFree);
  for (int i = 0; i < out_dims; ++i) {
    const int d1 = i >= dims1 ? 1 : SizeOfDimension(input1, dims1 - i - 1);
    const int d2 = i >= dims2 ? 1 : SizeOfDimension(input2, dims2 - i - 1);
    if (!(d1 == d2 || d1 == 1 || d2 == 1)) {
      context->ReportError(context,
                           "Given shapes, %s and %s, are not broadcastable.",
                           GetShapeDebugString(input1->dims).c_str(),
                           GetShapeDebugString(input2->dims).c_str());
      return kTfLiteError;
    }

    if (d1 == 0 || d2 == 0) {
      shape->data[out_dims - i - 1] = 0;
    } else {
      shape->data[out_dims - i - 1] = std::max(d1, d2);
    }
  }
  *output_shape = shape.release();
  return kTfLiteOk;
}