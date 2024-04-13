TfLiteStatus PopulateConvolutionQuantizationParams(
    TfLiteContext* context, const TfLiteTensor* input,
    const TfLiteTensor* filter, const TfLiteTensor* bias, TfLiteTensor* output,
    const TfLiteFusedActivation& activation, int32_t* multiplier, int* shift,
    int32_t* output_activation_min, int32_t* output_activation_max,
    int32_t* per_channel_multiplier, int32_t* per_channel_shift,
    int num_channels) {
  TF_LITE_ENSURE_EQ(context, input->quantization.type,
                    kTfLiteAffineQuantization);
  TF_LITE_ENSURE_EQ(context, filter->quantization.type,
                    kTfLiteAffineQuantization);
  // TODO(jianlijianli): Enable bias type check and bias scale == input scale
  // * filter scale for each channel in affine quantization once bias
  // quantization is properly populated.
  // TF_LITE_ENSURE_EQ(context, bias->quantization.type,
  // kTfLiteAffineQuantization);

  // Check data type.
  const auto* affine_quantization =
      reinterpret_cast<TfLiteAffineQuantization*>(filter->quantization.params);
  TF_LITE_ENSURE(context, affine_quantization);
  TF_LITE_ENSURE(context, affine_quantization->scale);
  const bool is_per_channel = affine_quantization->scale->size > 1;
  if (is_per_channel) {
    //  Currently only Int8/Int16 is supported for per channel quantization.
    TF_LITE_ENSURE(context,
                   input->type == kTfLiteInt8 || input->type == kTfLiteInt16);
    TF_LITE_ENSURE_EQ(context, filter->type, kTfLiteInt8);
    TF_LITE_ENSURE_EQ(context, affine_quantization->scale->size, num_channels);
    TF_LITE_ENSURE_EQ(
        context, num_channels,
        filter->dims->data[affine_quantization->quantized_dimension]);
  }

  // Populate multiplier and shift using affine quantization.
  const float input_scale = input->params.scale;
  const float output_scale = output->params.scale;
  const float* filter_scales = affine_quantization->scale->data;
  for (int i = 0; i < num_channels; ++i) {
    // If per-tensor quantization parameter is specified, broadcast it along the
    // quantization dimension (channels_out).
    const float scale = is_per_channel ? filter_scales[i] : filter_scales[0];
    const double filter_scale = static_cast<double>(scale);
    const double effective_output_scale = static_cast<double>(input_scale) *
                                          filter_scale /
                                          static_cast<double>(output_scale);
    int32_t significand;
    int channel_shift;
    QuantizeMultiplier(effective_output_scale, &significand, &channel_shift);
    per_channel_multiplier[i] = significand;
    per_channel_shift[i] = channel_shift;
  }

  // Populate scalar quantization parameters.
  // This check on legacy quantization parameters is kept only for backward
  // compatibility.
  if (input->type == kTfLiteUInt8) {
    // Check bias scale == input scale * filter scale.
    double real_multiplier = 0.0;
    TF_LITE_ENSURE_STATUS(GetQuantizedConvolutionMultipler(
        context, input, filter, bias, output, &real_multiplier));
    int exponent;

    // Populate quantization parameters with multiplier and shift.
    QuantizeMultiplier(real_multiplier, multiplier, &exponent);
    *shift = -exponent;
  }
  if (input->type == kTfLiteInt8 || input->type == kTfLiteUInt8 ||
      input->type == kTfLiteInt16) {
    TF_LITE_ENSURE_STATUS(CalculateActivationRangeQuantized(
        context, activation, output, output_activation_min,
        output_activation_max));
  }
  return kTfLiteOk;
}