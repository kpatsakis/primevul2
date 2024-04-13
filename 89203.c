void InputConnectionImpl::StartStateUpdateTimer() {
  state_update_timer_.Start(
      FROM_HERE, kStateUpdateTimeout,
      base::BindOnce(&InputConnectionImpl::UpdateTextInputState,
                     base::Unretained(this),
                     true /* is_input_state_update_requested */));
}
