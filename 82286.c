bool PrintWebViewHelper::SetOptionsFromPdfDocument(
    PrintHostMsg_SetOptionsFromDocument_Params* options) {
  blink::WebLocalFrame* source_frame = print_preview_context_.source_frame();
  const blink::WebNode& source_node = print_preview_context_.source_node();

  blink::WebPrintPresetOptions preset_options;
  if (!source_frame->getPrintPresetOptionsForPlugin(source_node,
                                                    &preset_options)) {
    return false;
  }

  options->is_scaling_disabled = PDFShouldDisableScalingBasedOnPreset(
      preset_options, print_pages_params_->params);
  options->copies = preset_options.copies;

  switch (preset_options.duplexMode) {
    case blink::WebSimplex:
      options->duplex = SIMPLEX;
      break;
    case blink::WebLongEdge:
      options->duplex = LONG_EDGE;
      break;
    default:
      options->duplex = UNKNOWN_DUPLEX_MODE;
      break;
  }
  return true;
}
