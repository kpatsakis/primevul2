DoHandleReverseCase(OpKernelContext* context, const Tensor& input,
                    Tensor* result) {
  if (sizeof(T) == 1) {
    static_assert(sizeof(uint8) == 1, "uint8 must be 1 byte.");
    ReverseRows<uint8, NUM_CHANNELS>(context, input, result);
  } else if (sizeof(T) == 2) {
    static_assert(sizeof(uint16) == 2, "uint16 must be 2 bytes");
    ReverseRows<uint16, NUM_CHANNELS>(context, input, result);
  } else if (sizeof(T) == 4) {
    static_assert(sizeof(uint32) == 4, "uint32 must be 4 bytes");
    ReverseRows<uint32, NUM_CHANNELS>(context, input, result);
  } else if (sizeof(T) == 8) {
    static_assert(sizeof(uint64) == 8, "uint64 must be 8 bytes");
    ReverseRows<uint64, NUM_CHANNELS>(context, input, result);
  } else if (sizeof(T) == 16) {
    static_assert(sizeof(complex128) == 16, "complex128 must be 16 bytes");
    ReverseRows<complex128, NUM_CHANNELS>(context, input, result);
  } else {
    context->CtxFailure(errors::InvalidArgument(DataTypeString(input.dtype()),
                                                " has unexpected size of ",
                                                sizeof(T), " bytes"));
  }
}