void PrepareFrameAndViewForPrint::CallOnReady() {
  return on_ready_.Run();  // Can delete |this|.
}
