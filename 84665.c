Browser::CreateParams Browser::CreateParams::CreateForDevTools(
    Profile* profile) {
  CreateParams params(TYPE_POPUP, profile);
  params.app_name = DevToolsWindow::kDevToolsApp;
  params.trusted_source = true;
  return params;
}
