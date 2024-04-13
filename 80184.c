bool ResourceDispatcherHostImpl::HandleExternalProtocol(ResourceLoader* loader,
                                                        const GURL& url) {
  if (!delegate_)
    return false;

  ResourceRequestInfoImpl* info = loader->GetRequestInfo();

  if (!ResourceType::IsFrame(info->GetResourceType()))
    return false;

  const net::URLRequestJobFactory* job_factory =
      info->GetContext()->GetRequestContext()->job_factory();
  if (job_factory->IsHandledURL(url))
    return false;

  return delegate_->HandleExternalProtocol(url, info->GetChildID(),
                                           info->GetRouteID());
}
