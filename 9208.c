TfLiteStatus GetQuantizedConvolutionMultipler(TfLiteContext* context,
                                              const TfLiteTensor* input,
                                              const TfLiteTensor* filter,
                                              const TfLiteTensor* bias,
                                              TfLiteTensor* output,
                                              double* multiplier) {
  const double input_product_scale = static_cast<double>(input->params.scale) *
                                     static_cast<double>(filter->params.scale);
  // The following conditions must be guaranteed by the training pipeline.
  if (bias) {
    const double bias_scale = static_cast<double>(bias->params.scale);
    // Here we're making sure the input_product_scale & bias_scale are about the
    // same. Since we have:
    // (output - output_zp) * output_scale =
    // input_product_scale * input_product + bias * bias_scale ---- (0)
    //
    // (0) equals:
    // (input_product + bias) * input_product_scale ----- (1)
    //           +
    // bias * (bias_scale - input_product_scale)   ------ (2)
    //
    // For the real kernel computation, we're doing (1), so we really need to
    // make sure (2) has minimum impact on the output, so:
    // bias * (bias_scale - input_product_scale) / output_scale should be
    // a small number for an integer.
    // Since normally bias should be within a small range.
    // We should expect (bias_scale - input_product_scale) / output_scale to
    // be a small number like 0.02.
    const double scale_diff = std::abs(input_product_scale - bias_scale);
    const double output_scale = static_cast<double>(output->params.scale);

    TF_LITE_ENSURE(context, scale_diff / output_scale <= 0.02);
  }
  return GetQuantizedConvolutionMultipler(context, input, filter, output,
                                          multiplier);
}