void ScopedSimulateModifierKeyPress::KeyPress(ui::DomKey key,
                                              ui::DomCode code,
                                              ui::KeyboardCode key_code) {
  SimulateKeyEvent(web_contents_, key, code, key_code, /*send_char=*/true,
                   modifiers_);
}
