xmlBuildRelativeURI (const xmlChar * URI, const xmlChar * base)
{
    xmlChar *val = NULL;
 int ret;
 int ix;
 int pos = 0;
 int nbslash = 0;
 int len;
    xmlURIPtr ref = NULL;
    xmlURIPtr bas = NULL;
    xmlChar *bptr, *uptr, *vptr;
 int remove_path = 0;

 if ((URI == NULL) || (*URI == 0))
 return NULL;

 /*
     * First parse URI into a standard form
     */
    ref = xmlCreateURI ();
 if (ref == NULL)
 return NULL;
 /* If URI not already in "relative" form */
 if (URI[0] != '.') {
	ret = xmlParseURIReference (ref, (const char *) URI);
 if (ret != 0)
 goto done; /* Error in URI, return NULL */
 } else
	ref->path = (char *)xmlStrdup(URI);

 /*
     * Next parse base into the same standard form
     */
 if ((base == NULL) || (*base == 0)) {
	val = xmlStrdup (URI);
 goto done;
 }
    bas = xmlCreateURI ();
 if (bas == NULL)
 goto done;
 if (base[0] != '.') {
	ret = xmlParseURIReference (bas, (const char *) base);
 if (ret != 0)
 goto done; /* Error in base, return NULL */
 } else
	bas->path = (char *)xmlStrdup(base);

 /*
     * If the scheme / server on the URI differs from the base,
     * just return the URI
     */
 if ((ref->scheme != NULL) &&
 ((bas->scheme == NULL) ||
 (xmlStrcmp ((xmlChar *)bas->scheme, (xmlChar *)ref->scheme)) ||
 (xmlStrcmp ((xmlChar *)bas->server, (xmlChar *)ref->server)))) {
	val = xmlStrdup (URI);
 goto done;
 }
 if (xmlStrEqual((xmlChar *)bas->path, (xmlChar *)ref->path)) {
	val = xmlStrdup(BAD_CAST "");
 goto done;
 }
 if (bas->path == NULL) {
	val = xmlStrdup((xmlChar *)ref->path);
 goto done;
 }
 if (ref->path == NULL) {
        ref->path = (char *) "/";
	remove_path = 1;
 }

 /*
     * At this point (at last!) we can compare the two paths
     *
     * First we take care of the special case where either of the
     * two path components may be missing (bug 316224)
     */
 if (bas->path == NULL) {
 if (ref->path != NULL) {
	    uptr = (xmlChar *) ref->path;
 if (*uptr == '/')
		uptr++;
 /* exception characters from xmlSaveUri */
	    val = xmlURIEscapeStr(uptr, BAD_CAST "/;&=+$,");
 }
 goto done;
 }
    bptr = (xmlChar *)bas->path;
 if (ref->path == NULL) {
 for (ix = 0; bptr[ix] != 0; ix++) {
 if (bptr[ix] == '/')
		nbslash++;
 }
	uptr = NULL;
	len = 1; /* this is for a string terminator only */
 } else {
 /*
     * Next we compare the two strings and find where they first differ
     */
 if ((ref->path[pos] == '.') && (ref->path[pos+1] == '/'))
            pos += 2;
 if ((*bptr == '.') && (bptr[1] == '/'))
            bptr += 2;
 else if ((*bptr == '/') && (ref->path[pos] != '/'))
	    bptr++;
 while ((bptr[pos] == ref->path[pos]) && (bptr[pos] != 0))
	    pos++;

 if (bptr[pos] == ref->path[pos]) {
	    val = xmlStrdup(BAD_CAST "");
 goto done; /* (I can't imagine why anyone would do this) */
 }

 /*
	 * In URI, "back up" to the last '/' encountered.  This will be the
	 * beginning of the "unique" suffix of URI
	 */
	ix = pos;
 if ((ref->path[ix] == '/') && (ix > 0))
	    ix--;
 else if ((ref->path[ix] == 0) && (ix > 1) && (ref->path[ix - 1] == '/'))
	    ix -= 2;
 for (; ix > 0; ix--) {
 if (ref->path[ix] == '/')
 break;
 }
 if (ix == 0) {
	    uptr = (xmlChar *)ref->path;
 } else {
	    ix++;
	    uptr = (xmlChar *)&ref->path[ix];
 }

 /*
	 * In base, count the number of '/' from the differing point
	 */
 if (bptr[pos] != ref->path[pos]) {/* check for trivial URI == base */
 for (; bptr[ix] != 0; ix++) {
 if (bptr[ix] == '/')
		    nbslash++;
 }
 }
	len = xmlStrlen (uptr) + 1;
 }

 if (nbslash == 0) {
 if (uptr != NULL)
 /* exception characters from xmlSaveUri */
	    val = xmlURIEscapeStr(uptr, BAD_CAST "/;&=+$,");
 goto done;
 }

 /*
     * Allocate just enough space for the returned string -
     * length of the remainder of the URI, plus enough space
     * for the "../" groups, plus one for the terminator
     */
    val = (xmlChar *) xmlMalloc (len + 3 * nbslash);
 if (val == NULL) {
        xmlURIErrMemory("building relative URI\n");
 goto done;
 }
    vptr = val;
 /*
     * Put in as many "../" as needed
     */
 for (; nbslash>0; nbslash--) {
 *vptr++ = '.';
 *vptr++ = '.';
 *vptr++ = '/';
 }
 /*
     * Finish up with the end of the URI
     */
 if (uptr != NULL) {
 if ((vptr > val) && (len > 0) &&
 (uptr[0] == '/') && (vptr[-1] == '/')) {
	    memcpy (vptr, uptr + 1, len - 1);
	    vptr[len - 2] = 0;
 } else {
	    memcpy (vptr, uptr, len);
	    vptr[len - 1] = 0;
 }
 } else {
	vptr[len - 1] = 0;
 }

 /* escape the freshly-built path */
    vptr = val;
 /* exception characters from xmlSaveUri */
    val = xmlURIEscapeStr(vptr, BAD_CAST "/;&=+$,");
    xmlFree(vptr);

done:
 /*
     * Free the working variables
     */
 if (remove_path != 0)
        ref->path = NULL;
 if (ref != NULL)
	xmlFreeURI (ref);
 if (bas != NULL)
	xmlFreeURI (bas);

 return val;
}
