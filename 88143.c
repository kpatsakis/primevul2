std::wstring GetRegistryPath() {
  std::wstring result(L"Software\\");
  AppendChromeInstallSubDirectory(InstallDetails::Get().mode(),
                                  true /* include_suffix */, &result);
  return result;
}
