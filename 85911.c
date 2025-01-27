void RenderWidgetHostImpl::RequestCompositorFrameSink(
    viz::mojom::CompositorFrameSinkRequest compositor_frame_sink_request,
    viz::mojom::CompositorFrameSinkClientPtr compositor_frame_sink_client) {
  if (enable_viz_) {
      auto callback = base::BindOnce(
          [](viz::HostFrameSinkManager* manager,
             viz::mojom::CompositorFrameSinkRequest request,
             viz::mojom::CompositorFrameSinkClientPtr client,
             const viz::FrameSinkId& frame_sink_id) {
            manager->CreateCompositorFrameSink(
                frame_sink_id, std::move(request), std::move(client));
          },
          base::Unretained(GetHostFrameSinkManager()),
          std::move(compositor_frame_sink_request),
          std::move(compositor_frame_sink_client));

      if (view_)
        std::move(callback).Run(view_->GetFrameSinkId());
      else
        create_frame_sink_callback_ = std::move(callback);

      return;
  }

  for (const auto& id : owned_bitmaps_)
    shared_bitmap_manager_->ChildDeletedSharedBitmap(id);
  owned_bitmaps_.clear();

  if (compositor_frame_sink_binding_.is_bound())
    compositor_frame_sink_binding_.Close();
  compositor_frame_sink_binding_.Bind(
      std::move(compositor_frame_sink_request),
      BrowserMainLoop::GetInstance()->GetResizeTaskRunner());
  if (view_) {
    view_->DidCreateNewRendererCompositorFrameSink(
        compositor_frame_sink_client.get());
  }
  renderer_compositor_frame_sink_ = std::move(compositor_frame_sink_client);
}
