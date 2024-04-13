void LayerWebKitThread::setNeedsDisplayInRect(const FloatRect& dirtyRect)
{
    if (m_tiler)
        m_tiler->setNeedsDisplay(dirtyRect);
    setNeedsCommit(); // FIXME: Replace this with a more targeted message for dirty rect handling with plugin content?
}
