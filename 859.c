void WasmBinaryBuilder::visitRethrow(Rethrow* curr) {
  BYN_TRACE("zz node: Rethrow\n");
  curr->target = getExceptionTargetName(getU32LEB());
  // This special target is valid only for delegates
  assert(curr->target != DELEGATE_CALLER_TARGET);
  curr->finalize();
}