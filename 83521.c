void BaseAudioContext::Initialize() {
  if (IsDestinationInitialized())
    return;

  FFTFrame::Initialize();

  audio_worklet_ = AudioWorklet::Create(this);

  if (destination_node_) {
    destination_node_->Handler().Initialize();
    destination_handler_ = &destination_node_->GetAudioDestinationHandler();

    listener_ = AudioListener::Create(*this);
  }
}
