    OVS_REQUIRES(ofproto_mutex)
{
    struct ofproto *ofproto = rule->ofproto;
    struct oftable *table = &ofproto->tables[rule->table_id];

    ovs_assert(!cls_rule_visible_in_version(&rule->cr, OVS_VERSION_MAX));
    if (!classifier_remove(&table->cls, &rule->cr)) {
        OVS_NOT_REACHED();
    }
    if (ofproto->ofproto_class->rule_delete) {
        ofproto->ofproto_class->rule_delete(rule);
    }
    ofproto_rule_unref(rule);
}
