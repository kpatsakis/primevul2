bool Document::isInInvisibleSubframe() const
{
    if (!ownerElement())
        return false; // this is the root element

    ASSERT(frame());
    return !frame()->ownerLayoutObject();
}
