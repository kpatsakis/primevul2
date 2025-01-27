void RenderProcessHostImpl::CreateMediaStreamDispatcherHost(
    MediaStreamManager* media_stream_manager,
    mojom::MediaStreamDispatcherHostRequest request) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  if (!media_stream_dispatcher_host_) {
    media_stream_dispatcher_host_.reset(
        new MediaStreamDispatcherHost(GetID(), media_stream_manager));
  }
  media_stream_dispatcher_host_->BindRequest(std::move(request));
}
