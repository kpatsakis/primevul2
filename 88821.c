void WebContentsImpl::UpdateWebContentsVisibility(bool visible) {
  if (!did_first_set_visible_) {
    if (visible) {
      WasShown();
      did_first_set_visible_ = true;
    }
    return;
  }
  if (visible == should_normally_be_visible_)
    return;

  if (visible)
    WasShown();
  else
    WasHidden();
}
