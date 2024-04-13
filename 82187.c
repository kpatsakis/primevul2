void ContainerNode::willRemoveChild(Node& child)
{
    ASSERT(child.parentNode() == this);
    ChildListMutationScope(*this).willRemoveChild(child);
    child.notifyMutationObserversNodeWillDetach();
    dispatchChildRemovalEvents(child);
    ChildFrameDisconnector(child).disconnect();

    ScriptForbiddenScope scriptForbiddenScope;
    EventDispatchForbiddenScope assertNoEventDispatch;
    document().nodeWillBeRemoved(child); // e.g. mutation event listener can create a new range.
}
