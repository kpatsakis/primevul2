void HTMLMediaElement::Dispose() {
  ClearMediaPlayerAndAudioSourceProviderClientWithoutLocking();
}
