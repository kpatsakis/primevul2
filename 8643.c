  void CalculateBestEqualitySplit(
      TTypes<float, 4>::ConstTensor stats_summary,
      const Eigen::VectorXf& total_grad, const Eigen::VectorXf& total_hess,
      const int32_t node_id, const int32_t feature_dims,
      const int32_t logits_dim, const int32_t hessian_dim,
      const int32_t num_buckets, const float l1, const float l2,
      float* best_gain, int32* best_bucket, int32* best_f_dim,
      string* best_split_type, Eigen::VectorXf* best_contrib_for_left,
      Eigen::VectorXf* best_contrib_for_right) {
    const string kEqualityDefaultRight =
        boosted_trees::SplitTypeWithDefault_Name(
            boosted_trees::EQUALITY_DEFAULT_RIGHT);
    for (int f_dim = 0; f_dim < feature_dims; ++f_dim) {
      for (int bucket = 0; bucket < num_buckets; ++bucket) {
        ConstVectorMap stats_vec(&stats_summary(node_id, f_dim, bucket, 0),
                                 logits_dim + hessian_dim);
        Eigen::VectorXf curr_grad = stats_vec.head(logits_dim);
        Eigen::VectorXf curr_hess = stats_vec.tail(hessian_dim);
        MaybeUpdateBestSplit(curr_grad, total_grad - curr_grad, curr_hess,
                             total_hess - curr_hess, logits_dim, bucket, f_dim,
                             l1, l2, kEqualityDefaultRight, best_gain,
                             best_bucket, best_f_dim, best_split_type,
                             best_contrib_for_left, best_contrib_for_right);
      }
    }
  }