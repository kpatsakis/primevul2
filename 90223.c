static struct node *lookup_child_by_name_locked(struct node *node, const char *name)
{
 for (node = node->child; node; node = node->next) {
 /* use exact string comparison, nodes that differ by case
         * must be considered distinct even if they refer to the same
         * underlying file as otherwise operations such as "mv x x"
         * will not work because the source and target nodes are the same. */
 if (!strcmp(name, node->name) && !node->deleted) {
 return node;
 }
 }
 return 0;
}
