    bool operator()(const StatsPartitionKey& a,
                    const StatsPartitionKey& b) const {
      if (a.node_id < b.node_id) {
        return true;
      }
      if ((a.node_id == b.node_id) && (a.feature_dim < b.feature_dim)) {
        return true;
      }
      if ((a.node_id == b.node_id) && (a.feature_dim == b.feature_dim) &&
          (a.bucket_id < b.bucket_id)) {
        return true;
      }
      return false;
    }