BrowserChildProcessHostImpl::TakeMetricsAllocator() {
  return std::move(metrics_allocator_);
}
