  ComparisonOpModel(const TensorData& input1, const TensorData& input2,
                    TensorType input_type, BuiltinOperator op) {
    input1_ = AddInput(input1);
    input2_ = AddInput(input2);
    output_ = AddOutput(TensorType_BOOL);
    ConfigureBuiltinOp(op);
    BuildInterpreter({GetShape(input1_), GetShape(input2_)});
  }