decode_ONFACT_RAW13_COPY_FIELD(const struct onf_action_copy_field *oacf,
                               enum ofp_version ofp_version OVS_UNUSED,
                               const struct vl_mff_map *vl_mff_map,
                               uint64_t *tlv_bitmap, struct ofpbuf *ofpacts)
{
    return decode_copy_field__(oacf->src_offset, oacf->dst_offset,
                               oacf->n_bits, oacf, oacf->len,
                               OBJECT_OFFSETOF(oacf, pad3), vl_mff_map,
                               tlv_bitmap, ofpacts);
}
