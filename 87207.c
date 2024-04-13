void NotifyProcessHostDisconnected(const ChildProcessData& data) {
  for (auto& observer : g_browser_child_process_observers.Get())
    observer.BrowserChildProcessHostDisconnected(data);
}
