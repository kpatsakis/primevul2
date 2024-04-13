void RenderFrameHostImpl::RenderProcessGone(SiteInstanceImpl* site_instance) {
  DCHECK_EQ(site_instance_.get(), site_instance);

  if (GetNavigationHandle())
    GetNavigationHandle()->set_net_error_code(net::ERR_ABORTED);
  ResetLoadingState();

  set_nav_entry_id(0);

  if (is_audible_)
    GetProcess()->OnMediaStreamRemoved();
}
