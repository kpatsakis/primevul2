void DownloadManagerImpl::InterceptNavigation(
    std::unique_ptr<ResourceRequest> resource_request,
    std::vector<GURL> url_chain,
    scoped_refptr<ResourceResponse> response,
    mojom::URLLoaderClientEndpointsPtr url_loader_client_endpoints,
    net::CertStatus cert_status,
    int frame_tree_node_id) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (!delegate_)
    return;

  const GURL& url = resource_request->url;
  const std::string& method = resource_request->method;
  ResourceRequestInfo::WebContentsGetter web_contents_getter =
      base::BindRepeating(&GetWebContents, ChildProcessHost::kInvalidUniqueID,
                          MSG_ROUTING_NONE, frame_tree_node_id);

  base::OnceCallback<void(bool /* download allowed */)>
      on_download_checks_done = base::BindOnce(
          &DownloadManagerImpl::InterceptNavigationOnChecksComplete,
          weak_factory_.GetWeakPtr(), web_contents_getter,
          std::move(resource_request), std::move(url_chain),
          std::move(response), cert_status,
          std::move(url_loader_client_endpoints));

  delegate_->CheckDownloadAllowed(web_contents_getter, url, method,
                                  std::move(on_download_checks_done));
}
