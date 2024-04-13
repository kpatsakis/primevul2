std::set<int32_t> AXTree::GetReverseRelations(ax::mojom::IntAttribute attr,
                                              int32_t dst_id) const {
  DCHECK(IsNodeIdIntAttribute(attr));

  const auto& attr_relations = int_reverse_relations_.find(attr);
  if (attr_relations != int_reverse_relations_.end()) {
    const auto& result = attr_relations->second.find(dst_id);
    if (result != attr_relations->second.end())
      return result->second;
  }
  return std::set<int32_t>();
}