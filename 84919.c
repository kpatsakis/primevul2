void PDFiumEngine::OnSingleClick(int page_index, int char_index) {
  SetSelecting(true);
  selection_.push_back(PDFiumRange(pages_[page_index], char_index, 0));
}