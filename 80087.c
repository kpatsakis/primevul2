void RenderViewImpl::didCommitProvisionalLoad(WebFrame* frame,
                                              bool is_new_navigation) {
  DocumentState* document_state =
      DocumentState::FromDataSource(frame->dataSource());
  NavigationState* navigation_state = document_state->navigation_state();

  if (document_state->commit_load_time().is_null())
    document_state->set_commit_load_time(Time::Now());

  if (document_state->must_reset_scroll_and_scale_state()) {
    webview()->resetScrollAndScaleState();
    document_state->set_must_reset_scroll_and_scale_state(false);
  }

  if (is_new_navigation) {
    UpdateSessionHistory(frame);

    page_id_ = next_page_id_++;

    if (GetLoadingUrl(frame) != GURL(kSwappedOutURL)) {
      history_list_offset_++;
      if (history_list_offset_ >= kMaxSessionHistoryEntries)
        history_list_offset_ = kMaxSessionHistoryEntries - 1;
      history_list_length_ = history_list_offset_ + 1;
      history_page_ids_.resize(history_list_length_, -1);
      history_page_ids_[history_list_offset_] = page_id_;
    }
  } else {
    if (navigation_state->pending_page_id() != -1 &&
        navigation_state->pending_page_id() != page_id_ &&
        !navigation_state->request_committed()) {
      UpdateSessionHistory(frame);
      page_id_ = navigation_state->pending_page_id();

      history_list_offset_ = navigation_state->pending_history_list_offset();

      DCHECK(history_list_length_ <= 0 ||
             history_list_offset_ < 0 ||
             history_list_offset_ >= history_list_length_ ||
             history_page_ids_[history_list_offset_] == page_id_);
    }
  }

  FOR_EACH_OBSERVER(RenderViewObserver, observers_,
                    DidCommitProvisionalLoad(frame, is_new_navigation));

  navigation_state->set_request_committed(true);

  UpdateURL(frame);

  completed_client_redirect_src_ = Referrer();

  UpdateEncoding(frame, frame->view()->pageEncoding().utf8());

  if (!frame->parent()) {  // Only for top frames.
    RenderThreadImpl* render_thread_impl = RenderThreadImpl::current();
    if (render_thread_impl) {  // Can be NULL in tests.
      render_thread_impl->histogram_customizer()->
          RenderViewNavigatedToHost(GURL(GetLoadingUrl(frame)).host(),
                                    g_view_map.Get().size());
    }
  }
}
