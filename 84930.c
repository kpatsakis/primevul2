pp::Resource PDFiumEngine::PrintPages(
    const PP_PrintPageNumberRange_Dev* page_ranges, uint32_t page_range_count,
    const PP_PrintSettings_Dev& print_settings) {
  if (HasPermission(PDFEngine::PERMISSION_PRINT_HIGH_QUALITY))
    return PrintPagesAsPDF(page_ranges, page_range_count, print_settings);
  else if (HasPermission(PDFEngine::PERMISSION_PRINT_LOW_QUALITY))
    return PrintPagesAsRasterPDF(page_ranges, page_range_count, print_settings);
  return pp::Resource();
}
