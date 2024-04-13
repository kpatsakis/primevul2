  void PopulateForm(const std::string& field_id) {
    std::string js("document.getElementById('" + field_id + "').focus();");
    ASSERT_TRUE(content::ExecuteScript(render_view_host(), js));

    SendKeyToPageAndWait(ui::VKEY_DOWN);
    SendKeyToPopupAndWait(ui::VKEY_DOWN);
    SendKeyToPopupAndWait(ui::VKEY_RETURN);
  }
