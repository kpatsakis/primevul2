    OVS_REQUIRES(ofproto_mutex)
{
    struct rule *rule = rule_collection_rules(&ofm->new_rules)[0];
    ofproto_flow_mod_revert(rule->ofproto, ofm);
}
