WebContentsImpl::~WebContentsImpl() {
  CHECK(!is_being_destroyed_);

  is_being_destroyed_ = true;

  CHECK(!is_notifying_observers_);

  rwh_input_event_router_.reset();

  for (auto& entry : binding_sets_)
    entry.second->CloseAllBindings();

  WebContentsImpl* outermost = GetOutermostWebContents();
  if (this != outermost && ContainsOrIsFocusedWebContents()) {
    outermost->SetAsFocusedWebContentsIfNecessary();
  }

  if (mouse_lock_widget_)
    mouse_lock_widget_->RejectMouseLockOrUnlockIfNecessary();

  for (FrameTreeNode* node : frame_tree_.Nodes()) {
    node->render_manager()->ClearRFHsPendingShutdown();
    node->render_manager()->ClearWebUIInstances();
  }

  for (RenderWidgetHostImpl* widget : created_widgets_)
    widget->DetachDelegate();
  created_widgets_.clear();

  if (dialog_manager_) {
    dialog_manager_->CancelDialogs(this, /*reset_state=*/true);
  }

  color_chooser_.reset();

  NotifyDisconnected();

  NotificationService::current()->Notify(
      NOTIFICATION_WEB_CONTENTS_DESTROYED,
      Source<WebContents>(this),
      NotificationService::NoDetails());

  frame_tree_.root()->ResetForNewProcess();
  GetRenderManager()->ResetProxyHosts();

  RenderFrameHostManager* root = GetRenderManager();

  root->current_frame_host()->SetRenderFrameCreated(false);
  root->current_frame_host()->ResetNavigationRequests();

  frame_tree_.root()->ResetNavigationRequest(true, true);
  if (root->speculative_frame_host()) {
    root->speculative_frame_host()->SetRenderFrameCreated(false);
    root->speculative_frame_host()->ResetNavigationRequests();
  }

#if BUILDFLAG(ENABLE_PLUGINS)
  pepper_playback_observer_.reset();
#endif  // defined(ENABLED_PLUGINS)

  for (auto& observer : observers_)
    observer.FrameDeleted(root->current_frame_host());

  for (auto& observer : observers_)
    observer.RenderViewDeleted(root->current_host());

  for (auto& observer : observers_)
    observer.WebContentsDestroyed();

  for (auto& observer : observers_)
    observer.ResetWebContents();

  SetDelegate(nullptr);
}
