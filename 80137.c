void BookmarksIOFunction::FileSelectionCanceled(void* params) {
  Release();  // Balanced in BookmarksIOFunction::SelectFile()
}
