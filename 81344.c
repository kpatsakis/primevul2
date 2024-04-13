void AsyncPixelTransfersCompletedQuery::Destroy(bool /* have_context */) {
  if (!IsDeleted()) {
    MarkAsDeleted();
  }
}
