static bool hasName(const HTMLToken& token, const QualifiedName& name)
{
    return threadSafeMatch(token.name(), name);
}
