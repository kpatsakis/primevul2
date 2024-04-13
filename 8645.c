  bool operator==(const StatsPartitionKey& other) const {
    return (node_id == other.node_id) && (feature_dim == other.feature_dim) &&
           (bucket_id == other.bucket_id);
  }