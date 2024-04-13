RenderFrameHostImpl::TakeNavigationHandleForSameDocumentCommit(
    const FrameHostMsg_DidCommitProvisionalLoad_Params& params) {
  bool is_browser_initiated = (params.nav_entry_id != 0);

  NavigationHandleImpl* navigation_handle =
      same_document_navigation_request_
          ? same_document_navigation_request_->navigation_handle()
          : nullptr;

  if (is_browser_initiated && navigation_handle &&
      navigation_handle->GetURL() == params.url) {
    std::unique_ptr<NavigationHandleImpl> result_navigation_handle =
        same_document_navigation_request_->TakeNavigationHandle();
    same_document_navigation_request_.reset();
    return result_navigation_handle;
  }

  bool is_renderer_initiated = true;
  int pending_nav_entry_id = 0;
  NavigationEntryImpl* pending_entry = NavigationEntryImpl::FromNavigationEntry(
      frame_tree_node()->navigator()->GetController()->GetPendingEntry());
  if (pending_entry && pending_entry->GetUniqueID() == params.nav_entry_id) {
    pending_nav_entry_id = params.nav_entry_id;
    is_renderer_initiated = pending_entry->is_renderer_initiated();
  }

  return NavigationHandleImpl::Create(
      params.url, params.redirects, frame_tree_node_, is_renderer_initiated,
      true /* was_within_same_document */, base::TimeTicks::Now(),
      pending_nav_entry_id,
      false,                  // started_from_context_menu
      CSPDisposition::CHECK,  // should_check_main_world_csp
      false,                  // is_form_submission
      base::nullopt,          // suggested_filename
      nullptr);               // navigation_ui_data
}
