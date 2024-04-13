void BookmarksIOFunction::MultiFilesSelected(
    const std::vector<base::FilePath>& files, void* params) {
  Release();  // Balanced in BookmarsIOFunction::SelectFile()
  NOTREACHED() << "Should not be able to select multiple files";
}
