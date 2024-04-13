  void MaybeUpdateBestSplit(const Eigen::VectorXf& grad_for_left,
                            const Eigen::VectorXf& grad_for_right,
                            const Eigen::VectorXf& hess_for_left,
                            const Eigen::VectorXf& hess_for_right,
                            const int32_t logits_dim, const int32_t bucket,
                            const int32_t f_dim, const float l1, const float l2,
                            const string split_type, float* best_gain,
                            int32* best_bucket, int32* best_f_dim,
                            string* best_split_type,
                            Eigen::VectorXf* best_contrib_for_left,
                            Eigen::VectorXf* best_contrib_for_right) {
    // Left child.
    Eigen::VectorXf contrib_for_left(logits_dim);
    float gain_for_left;
    CalculateWeightsAndGains(grad_for_left, hess_for_left, l1, l2,
                             &contrib_for_left, &gain_for_left);
    Eigen::VectorXf contrib_for_right(logits_dim);
    float gain_for_right;
    CalculateWeightsAndGains(grad_for_right, hess_for_right, l1, l2,
                             &contrib_for_right, &gain_for_right);
    if (GainIsLarger(gain_for_left + gain_for_right, *best_gain)) {
      *best_gain = gain_for_left + gain_for_right;
      *best_bucket = bucket;
      *best_f_dim = f_dim;
      *best_contrib_for_left = contrib_for_left;
      *best_contrib_for_right = contrib_for_right;
      *best_split_type = split_type;
    }
  }