void HTMLMediaElement::Dispose() {
  ClearMediaPlayerAndAudioSourceProviderClientWithoutLocking();
}