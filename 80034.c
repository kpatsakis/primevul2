void RenderViewImpl::OnSetRendererPrefs(
    const RendererPreferences& renderer_prefs) {
  double old_zoom_level = renderer_preferences_.default_zoom_level;
  renderer_preferences_ = renderer_prefs;
  UpdateFontRenderingFromRendererPrefs();

#if defined(USE_DEFAULT_RENDER_THEME) || defined(TOOLKIT_GTK)
  if (renderer_prefs.use_custom_colors) {
    WebColorName name = WebKit::WebColorWebkitFocusRingColor;
    WebKit::setNamedColors(&name, &renderer_prefs.focus_ring_color, 1);
    WebKit::setCaretBlinkInterval(renderer_prefs.caret_blink_interval);
#if defined(TOOLKIT_GTK)
    ui::NativeTheme::instance()->SetScrollbarColors(
        renderer_prefs.thumb_inactive_color,
        renderer_prefs.thumb_active_color,
        renderer_prefs.track_color);
#endif  // defined(TOOLKIT_GTK)

    if (webview()) {
#if defined(TOOLKIT_GTK)
      webview()->setScrollbarColors(
          renderer_prefs.thumb_inactive_color,
          renderer_prefs.thumb_active_color,
          renderer_prefs.track_color);
#endif  // defined(TOOLKIT_GTK)
      webview()->setSelectionColors(
          renderer_prefs.active_selection_bg_color,
          renderer_prefs.active_selection_fg_color,
          renderer_prefs.inactive_selection_bg_color,
          renderer_prefs.inactive_selection_fg_color);
      webview()->themeChanged();
    }
  }
#endif  // defined(USE_DEFAULT_RENDER_THEME) || defined(TOOLKIT_GTK)

  if (RenderThreadImpl::current())  // Will be NULL during unit tests.
    RenderThreadImpl::current()->SetFlingCurveParameters(
        renderer_prefs.touchpad_fling_profile,
        renderer_prefs.touchscreen_fling_profile);

  if (webview() && !webview()->mainFrame()->document().isPluginDocument() &&
      !ZoomValuesEqual(old_zoom_level,
                       renderer_preferences_.default_zoom_level) &&
      ZoomValuesEqual(webview()->zoomLevel(), old_zoom_level)) {
    webview()->setZoomLevel(false, renderer_preferences_.default_zoom_level);
    zoomLevelChanged();
  }
}
