Browser::DownloadClosePreventionType Browser::OkToCloseWithInProgressDownloads(
    int* num_downloads_blocking) const {
  DCHECK(num_downloads_blocking);
  *num_downloads_blocking = 0;

  if (!g_browser_process->profile_manager())
    return DOWNLOAD_CLOSE_OK;

  int total_download_count =
      DownloadCoreService::NonMaliciousDownloadCountAllProfiles();
  if (total_download_count == 0)
    return DOWNLOAD_CLOSE_OK;   // No downloads; can definitely close.

  int profile_window_count = 0;
  int total_window_count = 0;
  for (auto* browser : *BrowserList::GetInstance()) {
    if (browser == this || browser->IsAttemptingToCloseBrowser())
      continue;

    if (browser->profile() == profile())
      profile_window_count++;
    total_window_count++;
  }

  if (total_window_count == 0) {
    *num_downloads_blocking = total_download_count;
    return DOWNLOAD_CLOSE_BROWSER_SHUTDOWN;
  }

  DownloadCoreService* download_core_service =
      DownloadCoreServiceFactory::GetForBrowserContext(profile());
  if ((profile_window_count == 0) &&
      (download_core_service->NonMaliciousDownloadCount() > 0) &&
      profile()->IsOffTheRecord()) {
    *num_downloads_blocking =
        download_core_service->NonMaliciousDownloadCount();
    return DOWNLOAD_CLOSE_LAST_WINDOW_IN_INCOGNITO_PROFILE;
  }

  return DOWNLOAD_CLOSE_OK;
}
