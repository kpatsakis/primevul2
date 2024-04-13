std::wstring GetChromeInstallSubDirectory() {
  std::wstring result;
  AppendChromeInstallSubDirectory(InstallDetails::Get().mode(),
                                  true /* include_suffix */, &result);
  return result;
}
