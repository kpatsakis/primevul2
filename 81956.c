void Document::setBody(PassRefPtrWillBeRawPtr<HTMLElement> prpNewBody, ExceptionState& exceptionState)
{
    RefPtrWillBeRawPtr<HTMLElement> newBody = prpNewBody;

    if (!newBody) {
        exceptionState.throwDOMException(HierarchyRequestError, ExceptionMessages::argumentNullOrIncorrectType(1, "HTMLElement"));
        return;
    }
    if (!documentElement()) {
        exceptionState.throwDOMException(HierarchyRequestError, "No document element exists.");
        return;
    }

    if (!isHTMLBodyElement(*newBody) && !isHTMLFrameSetElement(*newBody)) {
        exceptionState.throwDOMException(HierarchyRequestError, "The new body element is of type '" + newBody->tagName() + "'. It must be either a 'BODY' or 'FRAMESET' element.");
        return;
    }

    HTMLElement* oldBody = body();
    if (oldBody == newBody)
        return;

    if (oldBody)
        documentElement()->replaceChild(newBody.release(), oldBody, exceptionState);
    else
        documentElement()->appendChild(newBody.release(), exceptionState);
}
