void NavigationRequest::OnRequestRedirected(
    const net::RedirectInfo& redirect_info,
    const scoped_refptr<network::ResourceResponse>& response) {
#if defined(OS_ANDROID)
  base::WeakPtr<NavigationRequest> this_ptr(weak_factory_.GetWeakPtr());

  bool should_override_url_loading = false;
  if (!GetContentClient()->browser()->ShouldOverrideUrlLoading(
          frame_tree_node_->frame_tree_node_id(), browser_initiated_,
          redirect_info.new_url, redirect_info.new_method,
          false, true, frame_tree_node_->IsMainFrame(),
          common_params_.transition, &should_override_url_loading)) {
    return;
  }

  if (!this_ptr)
    return;

  if (should_override_url_loading) {
    bool is_external_protocol =
        !GetContentClient()->browser()->IsHandledURL(common_params_.url);
    navigation_handle_->set_net_error_code(net::ERR_ABORTED);
    navigation_handle_->UpdateStateFollowingRedirect(
        redirect_info.new_url, redirect_info.new_method,
        GURL(redirect_info.new_referrer), is_external_protocol,
        response->head.headers, response->head.connection_info,
        base::Bind(&NavigationRequest::OnRedirectChecksComplete,
                   base::Unretained(this)));
    frame_tree_node_->ResetNavigationRequest(false, true);
    return;
  }
#endif
  if (!ChildProcessSecurityPolicyImpl::GetInstance()->CanRedirectToURL(
          redirect_info.new_url)) {
    DVLOG(1) << "Denied redirect for "
             << redirect_info.new_url.possibly_invalid_spec();
    navigation_handle_->set_net_error_code(net::ERR_UNSAFE_REDIRECT);
    frame_tree_node_->ResetNavigationRequest(false, true);
    return;
  }

  if (!browser_initiated_ && source_site_instance() &&
      !ChildProcessSecurityPolicyImpl::GetInstance()->CanRequestURL(
          source_site_instance()->GetProcess()->GetID(),
          redirect_info.new_url)) {
    DVLOG(1) << "Denied unauthorized redirect for "
             << redirect_info.new_url.possibly_invalid_spec();
    navigation_handle_->set_net_error_code(net::ERR_UNSAFE_REDIRECT);
    frame_tree_node_->ResetNavigationRequest(false, true);
    return;
  }

  if (redirect_info.new_method != "POST")
    common_params_.post_data = nullptr;

  if (request_params_.navigation_timing.redirect_start.is_null()) {
    request_params_.navigation_timing.redirect_start =
        request_params_.navigation_timing.fetch_start;
  }
  request_params_.navigation_timing.redirect_end = base::TimeTicks::Now();
  request_params_.navigation_timing.fetch_start = base::TimeTicks::Now();

  request_params_.redirect_response.push_back(response->head);
  request_params_.redirect_infos.push_back(redirect_info);

  request_params_.redirects.push_back(common_params_.url);
  common_params_.url = redirect_info.new_url;
  common_params_.method = redirect_info.new_method;
  common_params_.referrer.url = GURL(redirect_info.new_referrer);
  common_params_.referrer =
      Referrer::SanitizeForRequest(common_params_.url, common_params_.referrer);

  net::Error net_error = CheckContentSecurityPolicy(
      true /* is redirect */, redirect_info.insecure_scheme_was_upgraded,
      false /* is_response_check */);
  if (net_error != net::OK) {
    OnRequestFailedInternal(
        network::URLLoaderCompletionStatus(net_error), false /*skip_throttles*/,
        base::nullopt /*error_page_content*/, false /*collapse_frame*/);

    return;
  }

  if (CheckCredentialedSubresource() ==
          CredentialedSubresourceCheckResult::BLOCK_REQUEST ||
      CheckLegacyProtocolInSubresource() ==
          LegacyProtocolInSubresourceCheckResult::BLOCK_REQUEST) {
    OnRequestFailedInternal(
        network::URLLoaderCompletionStatus(net::ERR_ABORTED),
        false /*skip_throttles*/, base::nullopt /*error_page_content*/,
        false /*collapse_frame*/);

    return;
  }

  scoped_refptr<SiteInstance> site_instance =
      frame_tree_node_->render_manager()->GetSiteInstanceForNavigationRequest(
          *this);
  speculative_site_instance_ =
      site_instance->HasProcess() ? site_instance : nullptr;

  if (!site_instance->HasProcess()) {
    RenderProcessHostImpl::NotifySpareManagerAboutRecentlyUsedBrowserContext(
        site_instance->GetBrowserContext());
  }

  RenderProcessHost* expected_process =
      site_instance->HasProcess() ? site_instance->GetProcess() : nullptr;

  bool is_external_protocol =
      !GetContentClient()->browser()->IsHandledURL(common_params_.url);
  navigation_handle_->WillRedirectRequest(
      common_params_.url, common_params_.method, common_params_.referrer.url,
      is_external_protocol, response->head.headers,
      response->head.connection_info, expected_process,
      base::Bind(&NavigationRequest::OnRedirectChecksComplete,
                 base::Unretained(this)));
}
