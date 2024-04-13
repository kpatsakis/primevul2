  ComparisonOpModel(std::initializer_list<int> input1_shape,
                    std::initializer_list<int> input2_shape,
                    TensorType input_type, BuiltinOperator op) {
    input1_ = AddInput(input_type);
    input2_ = AddInput(input_type);
    output_ = AddOutput(TensorType_BOOL);
    ConfigureBuiltinOp(op);
    BuildInterpreter({input1_shape, input2_shape});
  }