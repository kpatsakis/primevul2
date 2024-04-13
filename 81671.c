bool XSSAuditor::filterStartToken(const FilterTokenRequest& request)
{
    m_state = FilteringTokens;
    bool didBlockScript = eraseDangerousAttributesIfInjected(request);

    if (hasName(request.token, scriptTag)) {
        didBlockScript |= filterScriptToken(request);
        ASSERT(request.shouldAllowCDATA || !m_scriptTagNestingLevel);
        m_scriptTagNestingLevel++;
    } else if (hasName(request.token, objectTag))
        didBlockScript |= filterObjectToken(request);
    else if (hasName(request.token, paramTag))
        didBlockScript |= filterParamToken(request);
    else if (hasName(request.token, embedTag))
        didBlockScript |= filterEmbedToken(request);
    else if (hasName(request.token, appletTag))
        didBlockScript |= filterAppletToken(request);
    else if (hasName(request.token, iframeTag) || hasName(request.token, frameTag))
        didBlockScript |= filterFrameToken(request);
    else if (hasName(request.token, metaTag))
        didBlockScript |= filterMetaToken(request);
    else if (hasName(request.token, baseTag))
        didBlockScript |= filterBaseToken(request);
    else if (hasName(request.token, formTag))
        didBlockScript |= filterFormToken(request);
    else if (hasName(request.token, inputTag))
        didBlockScript |= filterInputToken(request);
    else if (hasName(request.token, buttonTag))
        didBlockScript |= filterButtonToken(request);

    return didBlockScript;
}
