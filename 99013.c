    OVS_REQUIRES(ofproto_mutex)
{
    struct oftable *table = &rule->ofproto->tables[rule->table_id];
    const struct mf_subfield *sf;
    struct flow flow;
    uint32_t hash;

    hash = table->eviction_group_id_basis;
    miniflow_expand(rule->cr.match.flow, &flow);
    for (sf = table->eviction_fields;
         sf < &table->eviction_fields[table->n_eviction_fields];
         sf++)
    {
        if (mf_are_prereqs_ok(sf->field, &flow, NULL)) {
            union mf_value value;

            mf_get_value(sf->field, &flow, &value);
            if (sf->ofs) {
                bitwise_zero(&value, sf->field->n_bytes, 0, sf->ofs);
            }
            if (sf->ofs + sf->n_bits < sf->field->n_bytes * 8) {
                unsigned int start = sf->ofs + sf->n_bits;
                bitwise_zero(&value, sf->field->n_bytes, start,
                             sf->field->n_bytes * 8 - start);
            }
            hash = hash_bytes(&value, sf->field->n_bytes, hash);
        } else {
            hash = hash_int(hash, 0);
        }
    }

    return hash;
}
