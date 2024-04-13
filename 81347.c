void AsyncReadPixelsCompletedQuery::Destroy(bool /* have_context */) {
  if (!IsDeleted()) {
    MarkAsDeleted();
  }
}
