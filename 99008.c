    OVS_REQUIRES(ofproto_mutex)
{
    enum ofperr error = 0;
    struct rule_collection rules;
    unsigned int count = table->n_flows;
    unsigned int max_flows = table->max_flows;

    rule_collection_init(&rules);

    while (count-- > max_flows) {
        struct rule *rule;

        if (!choose_rule_to_evict(table, &rule)) {
            error = OFPERR_OFPFMFC_TABLE_FULL;
            break;
        } else {
            eviction_group_remove_rule(rule);
            rule_collection_add(&rules, rule);
        }
    }
    delete_flows__(&rules, OFPRR_EVICTION, NULL);

    return error;
}
