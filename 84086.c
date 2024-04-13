void FrameLoader::load(const FrameLoadRequest& passedRequest, FrameLoadType frameLoadType,
    HistoryItem* historyItem, HistoryLoadType historyLoadType)
{
    ASSERT(m_frame->document());

    if (!m_frame->isNavigationAllowed())
        return;

    if (m_inStopAllLoaders)
        return;

    if (m_frame->page()->defersLoading() && isBackForwardLoadType(frameLoadType)) {
        m_deferredHistoryLoad = DeferredHistoryLoad::create(passedRequest.resourceRequest(), historyItem, frameLoadType, historyLoadType);
        return;
    }

    FrameLoadRequest request(passedRequest);
    request.resourceRequest().setHasUserGesture(UserGestureIndicator::processingUserGesture());

    if (!prepareRequestForThisFrame(request))
        return;

    Frame* targetFrame = request.form() ? nullptr : m_frame->findFrameForNavigation(AtomicString(request.frameName()), *m_frame);

    if (isBackForwardLoadType(frameLoadType)) {
        ASSERT(historyItem);
        m_provisionalItem = historyItem;
    }

    if (targetFrame && targetFrame != m_frame) {
        bool wasInSamePage = targetFrame->page() == m_frame->page();

        request.setFrameName("_self");
        targetFrame->navigate(request);
        Page* page = targetFrame->page();
        if (!wasInSamePage && page)
            page->chromeClient().focus();
        return;
    }

    setReferrerForFrameRequest(request.resourceRequest(), request.getShouldSendReferrer(), request.originDocument());

    FrameLoadType newLoadType = (frameLoadType == FrameLoadTypeStandard) ?
        determineFrameLoadType(request) : frameLoadType;
    NavigationPolicy policy = navigationPolicyForRequest(request);
    if (shouldOpenInNewWindow(targetFrame, request, policy)) {
        if (policy == NavigationPolicyDownload) {
            client()->loadURLExternally(request.resourceRequest(), NavigationPolicyDownload, String(), false);
        } else {
            request.resourceRequest().setFrameType(WebURLRequest::FrameTypeAuxiliary);
            createWindowForRequest(request, *m_frame, policy, request.getShouldSendReferrer(), request.getShouldSetOpener());
        }
        return;
    }

    const KURL& url = request.resourceRequest().url();
    bool sameDocumentHistoryNavigation =
        isBackForwardLoadType(newLoadType) && historyLoadType == HistorySameDocumentLoad;
    bool sameDocumentNavigation = policy == NavigationPolicyCurrentTab
        && shouldPerformFragmentNavigation(
            request.form(), request.resourceRequest().httpMethod(), newLoadType, url);

    if (sameDocumentHistoryNavigation || sameDocumentNavigation) {
        ASSERT(historyItem || !sameDocumentHistoryNavigation);
        RefPtr<SerializedScriptValue> stateObject = sameDocumentHistoryNavigation ?
            historyItem->stateObject() : nullptr;

        if (!sameDocumentHistoryNavigation) {
            m_documentLoader->setNavigationType(determineNavigationType(
                newLoadType, false, request.triggeringEvent()));
            if (shouldTreatURLAsSameAsCurrent(url))
                newLoadType = FrameLoadTypeReplaceCurrentItem;
        }

        loadInSameDocument(url, stateObject, newLoadType, historyLoadType, request.clientRedirect(), request.originDocument());
        return;
    }

    startLoad(request, newLoadType, policy);
}
