void DelegatedFrameHost::ReturnSubscriberTexture(
    base::WeakPtr<DelegatedFrameHost> dfh,
    scoped_refptr<OwnedMailbox> subscriber_texture,
    const gpu::SyncToken& sync_token) {
  if (!subscriber_texture.get())
    return;
  if (!dfh)
    return;

  subscriber_texture->UpdateSyncToken(sync_token);

  if (dfh->frame_subscriber_ && subscriber_texture->texture_id())
    dfh->idle_frame_subscriber_textures_.push_back(subscriber_texture);
}