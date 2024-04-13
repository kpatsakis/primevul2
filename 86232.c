pp::VarDictionary PDFiumEngine::TraverseBookmarks(FPDF_BOOKMARK bookmark,
                                                  unsigned int depth) {
  pp::VarDictionary dict;
  base::string16 title;
  unsigned long buffer_size = FPDFBookmark_GetTitle(bookmark, nullptr, 0);
  if (buffer_size > 0) {
    PDFiumAPIStringBufferSizeInBytesAdapter<base::string16> api_string_adapter(
        &title, buffer_size, true);
    api_string_adapter.Close(FPDFBookmark_GetTitle(
        bookmark, api_string_adapter.GetData(), buffer_size));
  }
  dict.Set(pp::Var("title"), pp::Var(base::UTF16ToUTF8(title)));

  FPDF_DEST dest = FPDFBookmark_GetDest(doc_, bookmark);
  if (dest) {
    unsigned long page_index = FPDFDest_GetPageIndex(doc_, dest);
    if (page_index < pages_.size() &&
        base::IsValueInRangeForNumericType<int32_t>(page_index)) {
      dict.Set(pp::Var("page"), pp::Var(static_cast<int32_t>(page_index)));
      PDFiumPage::LinkTarget target;
      pages_[page_index]->GetPageYTarget(dest, &target);
      if (target.y_in_pixels)
        dict.Set(pp::Var("y"), pp::Var(target.y_in_pixels.value()));
    }
  } else {
    FPDF_ACTION action = FPDFBookmark_GetAction(bookmark);
    buffer_size = FPDFAction_GetURIPath(doc_, action, nullptr, 0);
    if (buffer_size > 0) {
      std::string uri;
      PDFiumAPIStringBufferAdapter<std::string> api_string_adapter(
          &uri, buffer_size, true);
      api_string_adapter.Close(FPDFAction_GetURIPath(
          doc_, action, api_string_adapter.GetData(), buffer_size));
      dict.Set(pp::Var("uri"), pp::Var(uri));
    }
  }

  pp::VarArray children;

  const unsigned int kMaxDepth = 128;
  if (depth < kMaxDepth) {
    int child_index = 0;
    std::set<FPDF_BOOKMARK> seen_bookmarks;
    for (FPDF_BOOKMARK child_bookmark =
             FPDFBookmark_GetFirstChild(doc_, bookmark);
         child_bookmark;
         child_bookmark = FPDFBookmark_GetNextSibling(doc_, child_bookmark)) {
      if (base::ContainsKey(seen_bookmarks, child_bookmark))
        break;

      seen_bookmarks.insert(child_bookmark);
      children.Set(child_index, TraverseBookmarks(child_bookmark, depth + 1));
      child_index++;
    }
  }
  dict.Set(pp::Var("children"), children);
  return dict;
}
