 void FormAssociatedElement::formRemovedFromTree(const Node* formRoot)
 {
     ASSERT(m_form);
    if (toHTMLElement(this)->highestAncestor() == formRoot)
        return;
    RefPtr<HTMLElement> protector(toHTMLElement(this));
    setForm(0);
 }
