static void derive_permissions_locked(struct fuse* fuse, struct node *parent,
 struct node *node) {
 appid_t appid;

 /* By default, each node inherits from its parent */
    node->perm = PERM_INHERIT;
    node->userid = parent->userid;
    node->uid = parent->uid;
    node->under_android = parent->under_android;

 /* Derive custom permissions based on parent and current node */
 switch (parent->perm) {
 case PERM_INHERIT:
 /* Already inherited above */
 break;
 case PERM_PRE_ROOT:
 /* Legacy internal layout places users at top level */
        node->perm = PERM_ROOT;
        node->userid = strtoul(node->name, NULL, 10);
 break;
 case PERM_ROOT:
 /* Assume masked off by default. */
 if (!strcasecmp(node->name, "Android")) {
 /* App-specific directories inside; let anyone traverse */
            node->perm = PERM_ANDROID;
            node->under_android = true;
 }
 break;
 case PERM_ANDROID:
 if (!strcasecmp(node->name, "data")) {
 /* App-specific directories inside; let anyone traverse */
            node->perm = PERM_ANDROID_DATA;
 } else if (!strcasecmp(node->name, "obb")) {
 /* App-specific directories inside; let anyone traverse */
            node->perm = PERM_ANDROID_OBB;
 /* Single OBB directory is always shared */
            node->graft_path = fuse->global->obb_path;
            node->graft_pathlen = strlen(fuse->global->obb_path);
 } else if (!strcasecmp(node->name, "media")) {
 /* App-specific directories inside; let anyone traverse */
            node->perm = PERM_ANDROID_MEDIA;
 }
 break;
 case PERM_ANDROID_DATA:
 case PERM_ANDROID_OBB:
 case PERM_ANDROID_MEDIA:
        appid = (appid_t) (uintptr_t) hashmapGet(fuse->global->package_to_appid, node->name);
 if (appid != 0) {
            node->uid = multiuser_get_uid(parent->userid, appid);
 }
 break;
 }
}
