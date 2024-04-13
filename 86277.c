 void ChromeDownloadManagerDelegate::ShowDownloadInShell(
     DownloadItem* download) {
   if (!download->CanShowInFolder())
    return;

  MaybeSendDangerousDownloadOpenedReport(download,
                                         true /* show_download_in_folder */);

  base::FilePath platform_path(
      GetPlatformDownloadPath(profile_, download, PLATFORM_CURRENT_PATH));
  DCHECK(!platform_path.empty());
  platform_util::ShowItemInFolder(profile_, platform_path);
}
