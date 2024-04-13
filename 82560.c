void OutOfProcessInstance::HandleMessage(const pp::Var& message) {
  pp::VarDictionary dict(message);
  if (!dict.Get(kType).is_string()) {
    NOTREACHED();
    return;
  }

  std::string type = dict.Get(kType).AsString();

  if (type == kJSViewportType &&
      dict.Get(pp::Var(kJSXOffset)).is_number() &&
      dict.Get(pp::Var(kJSYOffset)).is_number() &&
      dict.Get(pp::Var(kJSZoom)).is_number()) {
    received_viewport_message_ = true;
    stop_scrolling_ = false;
    double zoom = dict.Get(pp::Var(kJSZoom)).AsDouble();
    pp::FloatPoint scroll_offset(dict.Get(pp::Var(kJSXOffset)).AsDouble(),
                                 dict.Get(pp::Var(kJSYOffset)).AsDouble());

    zoom = std::max(kMinZoom, zoom);
    SetZoom(zoom);
    scroll_offset = BoundScrollOffsetToDocument(scroll_offset);
    engine_->ScrolledToXPosition(scroll_offset.x() * device_scale_);
    engine_->ScrolledToYPosition(scroll_offset.y() * device_scale_);
  } else if (type == kJSGetPasswordCompleteType &&
             dict.Get(pp::Var(kJSPassword)).is_string()) {
    if (password_callback_) {
      pp::CompletionCallbackWithOutput<pp::Var> callback = *password_callback_;
      password_callback_.reset();
      *callback.output() = dict.Get(pp::Var(kJSPassword)).pp_var();
      callback.Run(PP_OK);
    } else {
      NOTREACHED();
    }
  } else if (type == kJSPrintType) {
    Print();
  } else if (type == kJSSaveType) {
    pp::PDF::SaveAs(this);
  } else if (type == kJSRotateClockwiseType) {
    RotateClockwise();
  } else if (type == kJSRotateCounterclockwiseType) {
    RotateCounterclockwise();
  } else if (type == kJSSelectAllType) {
    engine_->SelectAll();
  } else if (type == kJSResetPrintPreviewModeType &&
             dict.Get(pp::Var(kJSPrintPreviewUrl)).is_string() &&
             dict.Get(pp::Var(kJSPrintPreviewGrayscale)).is_bool() &&
             dict.Get(pp::Var(kJSPrintPreviewPageCount)).is_int()) {
    url_ = dict.Get(pp::Var(kJSPrintPreviewUrl)).AsString();
    preview_pages_info_ = std::queue<PreviewPageInfo>();
    preview_document_load_state_ = LOAD_STATE_COMPLETE;
    document_load_state_ = LOAD_STATE_LOADING;
    LoadUrl(url_);
    preview_engine_.reset();
    engine_.reset(PDFEngine::Create(this));
    engine_->SetGrayscale(dict.Get(pp::Var(kJSPrintPreviewGrayscale)).AsBool());
    engine_->New(url_.c_str(), nullptr /* empty header */);

    print_preview_page_count_ =
        std::max(dict.Get(pp::Var(kJSPrintPreviewPageCount)).AsInt(), 0);

    paint_manager_.InvalidateRect(pp::Rect(pp::Point(), plugin_size_));
  } else if (type == kJSLoadPreviewPageType &&
             dict.Get(pp::Var(kJSPreviewPageUrl)).is_string() &&
             dict.Get(pp::Var(kJSPreviewPageIndex)).is_int()) {
    ProcessPreviewPageInfo(dict.Get(pp::Var(kJSPreviewPageUrl)).AsString(),
                           dict.Get(pp::Var(kJSPreviewPageIndex)).AsInt());
  } else if (type == kJSGetAccessibilityJSONType) {
    pp::VarDictionary reply;
    reply.Set(pp::Var(kType), pp::Var(kJSGetAccessibilityJSONReplyType));
    if (dict.Get(pp::Var(kJSAccessibilityPageNumber)).is_int()) {
      int page = dict.Get(pp::Var(kJSAccessibilityPageNumber)).AsInt();
      reply.Set(pp::Var(kJSAccessibilityJSON),
                        pp::Var(engine_->GetPageAsJSON(page)));
    } else {
      base::DictionaryValue node;
      node.SetInteger(kAccessibleNumberOfPages, engine_->GetNumberOfPages());
      node.SetBoolean(kAccessibleLoaded,
                      document_load_state_ != LOAD_STATE_LOADING);
      bool has_permissions =
          engine_->HasPermission(PDFEngine::PERMISSION_COPY) ||
          engine_->HasPermission(PDFEngine::PERMISSION_COPY_ACCESSIBLE);
      node.SetBoolean(kAccessibleCopyable, has_permissions);
      std::string json;
      base::JSONWriter::Write(node, &json);
      reply.Set(pp::Var(kJSAccessibilityJSON), pp::Var(json));
    }
    PostMessage(reply);
  } else if (type == kJSStopScrollingType) {
    stop_scrolling_ = true;
  } else if (type == kJSGetSelectedTextType) {
    std::string selected_text = engine_->GetSelectedText();
    base::ReplaceChars(selected_text, "\r", std::string(), &selected_text);
    pp::VarDictionary reply;
    reply.Set(pp::Var(kType), pp::Var(kJSGetSelectedTextReplyType));
    reply.Set(pp::Var(kJSSelectedText), selected_text);
    PostMessage(reply);
  } else if (type == KJSGetNamedDestinationType &&
             dict.Get(pp::Var(KJSGetNamedDestination)).is_string()) {
    int page_number = engine_->GetNamedDestinationPage(
        dict.Get(pp::Var(KJSGetNamedDestination)).AsString());
    pp::VarDictionary reply;
    reply.Set(pp::Var(kType), pp::Var(kJSGetNamedDestinationReplyType));
    if (page_number >= 0)
      reply.Set(pp::Var(kJSNamedDestinationPageNumber), page_number);
    PostMessage(reply);
  } else {
    NOTREACHED();
  }
}
