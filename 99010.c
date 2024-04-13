    OVS_REQUIRES(ofproto_mutex)
{
    struct rule *rule;

    RULE_COLLECTION_FOR_EACH (rule, rules) {
        enum nx_flow_monitor_flags flags = rule->monitor_flags;
        rule->monitor_flags = 0;

        ofproto_compose_flow_refresh_update(rule, flags, msgs,
                ofproto_get_tun_tab(rule->ofproto));
    }
}
