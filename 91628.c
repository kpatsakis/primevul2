 explicit SloppyArgumentsElementsAccessor(const char* name)
 : ElementsAccessorBase<Subclass, KindTraits>(name) {
    USE(KindTraits::Kind);
 }
