  bool operator==(const InstanceFeatureDimKey& other) const {
    return (instance == other.instance) && (feature_dim == other.feature_dim);
  }