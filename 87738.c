bool OmniboxViewViews::IsCommandIdEnabled(int command_id) const {
  if (command_id == IDS_APP_PASTE)
    return !read_only() && !GetClipboardText().empty();
  if (command_id == IDS_PASTE_AND_GO)
    return !read_only() && model()->CanPasteAndGo(GetClipboardText());
  return Textfield::IsCommandIdEnabled(command_id) ||
         location_bar_view_->command_updater()->IsCommandEnabled(command_id);
}
