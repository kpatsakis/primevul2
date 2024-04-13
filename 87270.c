size_t RenderProcessHost::GetMaxRendererProcessCount() {
  if (g_max_renderer_count_override)
    return g_max_renderer_count_override;

#if defined(OS_ANDROID)
  static const size_t kNumRendererSlots =
      ChildProcessLauncher::GetNumberOfRendererSlots();
  return kNumRendererSlots;
#endif
#if defined(OS_CHROMEOS)
  return 100;
#endif


  static size_t max_count = 0;
  if (!max_count) {
    const size_t kEstimatedWebContentsMemoryUsage =
#if defined(ARCH_CPU_64_BITS)
        60;  // In MB
#else
        40;  // In MB
#endif
    max_count = base::SysInfo::AmountOfPhysicalMemoryMB() / 2;
    max_count /= kEstimatedWebContentsMemoryUsage;

    const size_t kMinRendererProcessCount = 3;
    max_count = std::max(max_count, kMinRendererProcessCount);
    max_count = std::min(max_count, kMaxRendererProcessCount);
  }
  return max_count;
}
