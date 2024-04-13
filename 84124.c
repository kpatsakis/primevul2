ResourceRequestInfoImpl* ResourceDispatcherHostImpl::CreateRequestInfo(
    int child_id,
    int render_view_route_id,
    int render_frame_route_id,
    bool download,
    ResourceContext* context) {
  return new ResourceRequestInfoImpl(
      PROCESS_TYPE_RENDERER,
      child_id,
      render_view_route_id,
      -1,  // frame_tree_node_id
      0,
      request_id_,
      render_frame_route_id,
      false,             // is_main_frame
      false,             // parent_is_main_frame
      RESOURCE_TYPE_SUB_RESOURCE,
      ui::PAGE_TRANSITION_LINK,
      false,     // should_replace_current_entry
      download,  // is_download
      false,     // is_stream
      download,  // allow_download
      false,     // has_user_gesture
      false,     // enable_load_timing
      false,     // enable_upload_progress
      false,     // do_not_prompt_for_login
      blink::WebReferrerPolicyDefault,
      blink::WebPageVisibilityStateVisible,
      context,
      base::WeakPtr<ResourceMessageFilter>(),  // filter
      false,                                   // report_raw_headers
      true,                                    // is_async
      false,                                   // is_using_lofi
      std::string());                          // original_headers
}
