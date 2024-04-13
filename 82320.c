RenderThreadImpl::SharedMainThreadContextProvider() {
  DCHECK(IsMainThread());
  if (!shared_main_thread_contexts_.get() ||
      shared_main_thread_contexts_->DestroyedOnMainThread()) {
    shared_main_thread_contexts_ = NULL;
#if defined(OS_ANDROID)
    if (SynchronousCompositorFactory* factory =
            SynchronousCompositorFactory::GetInstance()) {
      shared_main_thread_contexts_ = factory->CreateOffscreenContextProvider(
          GetOffscreenAttribs(), "Offscreen-MainThread");
    }
#endif
    if (!shared_main_thread_contexts_.get()) {
      shared_main_thread_contexts_ = ContextProviderCommandBuffer::Create(
          CreateOffscreenContext3d(), "Offscreen-MainThread");
    }
    if (shared_main_thread_contexts_.get() &&
        !shared_main_thread_contexts_->BindToCurrentThread())
      shared_main_thread_contexts_ = NULL;
  }
  return shared_main_thread_contexts_;
}
