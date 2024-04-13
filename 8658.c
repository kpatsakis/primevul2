    bool operator()(const InstanceFeatureDimKey& a,
                    const InstanceFeatureDimKey& b) const {
      if (a.instance < b.instance) {
        return true;
      }
      if ((a.instance == b.instance) && (a.feature_dim < b.feature_dim)) {
        return true;
      }
      return false;
    }