void NavigationControllerImpl::RendererDidNavigateNewSubframe(
    RenderFrameHostImpl* rfh,
    const FrameHostMsg_DidCommitProvisionalLoad_Params& params,
    bool is_same_document,
    bool replace_entry) {
  DCHECK(ui::PageTransitionCoreTypeIs(params.transition,
                                      ui::PAGE_TRANSITION_MANUAL_SUBFRAME));

  DCHECK(GetLastCommittedEntry()) << "ClassifyNavigation should guarantee "
                                  << "that a last committed entry exists.";

  DCHECK(!replace_entry);

  scoped_refptr<FrameNavigationEntry> frame_entry(new FrameNavigationEntry(
      rfh->frame_tree_node()->unique_name(), params.item_sequence_number,
      params.document_sequence_number, rfh->GetSiteInstance(), nullptr,
      params.url, params.referrer, params.redirects, params.page_state,
      params.method, params.post_id, nullptr /* blob_url_loader_factory */));

  std::unique_ptr<NavigationEntryImpl> new_entry =
      GetLastCommittedEntry()->CloneAndReplace(
          frame_entry.get(), is_same_document, rfh->frame_tree_node(),
          delegate_->GetFrameTree()->root());


  InsertOrReplaceEntry(std::move(new_entry), replace_entry);
}
