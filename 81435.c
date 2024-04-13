void RenderWidgetHostViewAura::AcceleratedSurfaceRelease() {
  if (current_surface_.get()) {
    ui::Compositor* compositor = GetCompositor();
    if (compositor) {
      AddOnCommitCallbackAndDisableLocks(
          base::Bind(&RenderWidgetHostViewAura::SetSurfaceNotInUseByCompositor,
                     AsWeakPtr(),
                     current_surface_));  // Hold a ref so the texture will not
    }
    current_surface_ = NULL;
    UpdateExternalTexture();
  }
}
