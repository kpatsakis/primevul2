htmlParseDoc(const xmlChar *cur, const char *encoding) {
    return(htmlSAXParseDoc(cur, encoding, NULL, NULL));
}
