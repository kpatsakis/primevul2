const GURL& DownloadItemImpl::GetOriginalUrl() const {
  return request_info_.url_chain.empty() ? GURL::EmptyGURL()
                                         : request_info_.url_chain.front();
}
