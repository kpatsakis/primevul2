void CommandsIssuedQuery::Destroy(bool /* have_context */) {
  if (!IsDeleted()) {
    MarkAsDeleted();
  }
}
