void Document::checkLoadEventSoon()
{
    if (frame() && !m_loadEventDelayTimer.isActive())
        m_loadEventDelayTimer.startOneShot(0, BLINK_FROM_HERE);
}
