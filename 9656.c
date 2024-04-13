  void ConfigureBuiltinOp(BuiltinOperator op) {
    switch (op) {
      case BuiltinOperator_EQUAL: {
        SetBuiltinOp(op, BuiltinOptions_EqualOptions,
                     CreateEqualOptions(builder_).Union());
        break;
      }
      case BuiltinOperator_NOT_EQUAL: {
        SetBuiltinOp(op, BuiltinOptions_NotEqualOptions,
                     CreateNotEqualOptions(builder_).Union());
        break;
      }
      case BuiltinOperator_GREATER: {
        SetBuiltinOp(op, BuiltinOptions_GreaterOptions,
                     CreateGreaterOptions(builder_).Union());
        break;
      }
      case BuiltinOperator_GREATER_EQUAL: {
        SetBuiltinOp(op, BuiltinOptions_GreaterEqualOptions,
                     CreateGreaterEqualOptions(builder_).Union());
        break;
      }
      case BuiltinOperator_LESS: {
        SetBuiltinOp(op, BuiltinOptions_LessOptions,
                     CreateLessOptions(builder_).Union());
        break;
      }
      case BuiltinOperator_LESS_EQUAL: {
        SetBuiltinOp(op, BuiltinOptions_LessEqualOptions,
                     CreateLessEqualOptions(builder_).Union());
        break;
      }
      default: { FAIL() << "We shouldn't get here."; }
    }
  }