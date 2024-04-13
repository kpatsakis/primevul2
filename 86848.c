void MimeHandlerViewContainer::DidFinishLoading(double /* unused */) {
  DCHECK(is_embedded_);
  CreateMimeHandlerViewGuestIfNecessary();
}
