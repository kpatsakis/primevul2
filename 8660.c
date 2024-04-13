  void CalculateBestInequalitySplit(
      TTypes<float, 4>::ConstTensor stats_summary, const int32_t node_id,
      const int32_t feature_dims, const int32_t logits_dim,
      const int32_t hessian_dim, const int32_t num_buckets,
      const float min_node_weight, const float l1, const float l2,
      float* best_gain, int32* best_bucket, int32* best_f_dim,
      string* best_split_type, Eigen::VectorXf* best_contrib_for_left,
      Eigen::VectorXf* best_contrib_for_right) {
    std::vector<Eigen::VectorXf> cum_grad;
    std::vector<Eigen::VectorXf> cum_hess;
    // get all cumulative gradients including default bucket.
    cum_grad.reserve(num_buckets);
    cum_hess.reserve(num_buckets);

    for (int f_dim = 0; f_dim < feature_dims; ++f_dim) {
      ConstVectorMap default_stats_vec(
          &stats_summary(node_id, f_dim, num_buckets, 0),
          logits_dim + hessian_dim);
      Eigen::VectorXf missing_bucket_grad = default_stats_vec.head(logits_dim);
      Eigen::VectorXf missing_bucket_hess = default_stats_vec.tail(hessian_dim);
      cum_grad.clear();
      cum_hess.clear();
      Eigen::VectorXf total_grad = Eigen::VectorXf::Zero(logits_dim);
      Eigen::VectorXf total_hess = Eigen::VectorXf::Zero(hessian_dim);
      // sum all the gradients including default bucket.
      for (int bucket = 0; bucket <= num_buckets; ++bucket) {
        for (int i = 0; i < logits_dim; ++i) {
          total_grad[i] += stats_summary(node_id, f_dim, bucket, i);
        }
        for (int i = 0; i < hessian_dim; ++i) {
          // Full hessian.
          total_hess[i] +=
              stats_summary(node_id, f_dim, bucket, logits_dim + i);
        }
        if (bucket < num_buckets) {
          cum_grad.push_back(total_grad);
          cum_hess.push_back(total_hess);
        }
      }
      const string kInequalityDefaultLeft =
          boosted_trees::SplitTypeWithDefault_Name(
              boosted_trees::INEQUALITY_DEFAULT_LEFT);
      const string kInequalityDefaultRight =
          boosted_trees::SplitTypeWithDefault_Name(
              boosted_trees::INEQUALITY_DEFAULT_RIGHT);

      // Iterate from left to right, excluding default bucket.
      for (int bucket = 0; bucket < num_buckets; ++bucket) {
        // default value goes to left node.
        const Eigen::VectorXf total_left_grad =
            cum_grad[bucket] + missing_bucket_grad;
        const Eigen::VectorXf total_left_hess =
            cum_hess[bucket] + missing_bucket_hess;
        MaybeUpdateBestSplit(
            total_left_grad, total_grad - total_left_grad, total_left_hess,
            total_hess - total_left_hess, logits_dim, bucket, f_dim, l1, l2,
            kInequalityDefaultLeft, best_gain, best_bucket, best_f_dim,
            best_split_type, best_contrib_for_left, best_contrib_for_right);
        // default value goes to right node.
        MaybeUpdateBestSplit(
            cum_grad[bucket], total_grad - cum_grad[bucket], cum_hess[bucket],
            total_hess - cum_hess[bucket], logits_dim, bucket, f_dim, l1, l2,
            kInequalityDefaultRight, best_gain, best_bucket, best_f_dim,
            best_split_type, best_contrib_for_left, best_contrib_for_right);
      }  // for bucket
    }
  }