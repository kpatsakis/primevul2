  void process_node(const FeatureMap& f_map,
                    std::vector<int32>* output_node_ids,
                    std::vector<float>* output_gains,
                    std::vector<int32>* output_feature_dimensions,
                    std::vector<int32>* output_thresholds,
                    std::vector<float>* output_left_node_contribs,
                    std::vector<float>* output_right_node_contribs,
                    std::vector<string>* output_split_types,
                    const int32_t node_id, const float min_node_weight,
                    const float l1, const float l2, const int32_t num_buckets) {
    float parent_gain;
    Eigen::VectorXf unused(logits_dim_);
    Eigen::MatrixXf identity;
    identity.setIdentity(1, 1);

    // start processing for previous node id.
    float best_gain = std::numeric_limits<float>::lowest();
    float best_bucket = 0;
    float best_f_dim = 0;
    string best_split_type = boosted_trees::SplitTypeWithDefault_Name(
        boosted_trees::INEQUALITY_DEFAULT_LEFT);
    float best_contrib_for_left = 0.0;
    float best_contrib_for_right = 0.0;
    // the sum of gradients including default bucket.
    float total_grad = 0;
    // the sum of hessians including default bucket.
    float total_hess = 0;

    for (auto f_iter = f_map.begin(); f_iter != f_map.end(); ++f_iter) {
      const int32_t feature_dim = f_iter->first;
      const auto buckets_to_stats_map = f_iter->second;

      // The very last bucket contains stats for missing values.
      // TODO(crawles): use vector for multi-class.
      const float default_grad =
          (buckets_to_stats_map.find(num_buckets) == buckets_to_stats_map.end()
               ? 0
               : buckets_to_stats_map.at(num_buckets)[0]);
      const float default_hess =
          (buckets_to_stats_map.find(num_buckets) == buckets_to_stats_map.end()
               ? 0
               : buckets_to_stats_map.at(num_buckets)[1]);

      if (f_iter == f_map.begin()) {
        // first get the sum of grads, including default bucket.
        for (auto b_iter = buckets_to_stats_map.begin();
             b_iter != buckets_to_stats_map.end(); ++b_iter) {
          total_grad += b_iter->second[0];
          total_hess += b_iter->second[1];
        }
        if (total_hess < min_node_weight) {
          // Do not split the node because not enough avg hessian.
          break;
        }
        CalculateWeightsAndGains(total_grad * identity, total_hess * identity,
                                 l1, l2, &unused, &parent_gain);
      }

      float total_left_grad = 0;
      float total_left_hess = 0;
      for (auto b_iter = buckets_to_stats_map.begin();
           b_iter != buckets_to_stats_map.end(); ++b_iter) {
        const int32_t bucket_id = b_iter->first;
        // total_left_stats should exclude stats from default bucket.
        if (bucket_id == num_buckets) {
          break;
        }
        // TODO(crawles): vector for multi-class.
        total_left_grad += b_iter->second[0];
        total_left_hess += b_iter->second[1];
        // From left to right, default right.
        // Left child.
        Eigen::VectorXf contrib_for_left(1);
        float gain_for_left;
        CalculateWeightsAndGains(total_left_grad * identity,
                                 total_left_hess * identity, l1, l2,
                                 &contrib_for_left, &gain_for_left);
        // Right child.
        Eigen::VectorXf contrib_for_right(1);
        float gain_for_right;
        CalculateWeightsAndGains((total_grad - total_left_grad) * identity,
                                 (total_hess - total_left_hess) * identity, l1,
                                 l2, &contrib_for_right, &gain_for_right);
        if (GainIsLarger(gain_for_left + gain_for_right, best_gain)) {
          best_gain = gain_for_left + gain_for_right;
          best_bucket = bucket_id;
          best_f_dim = feature_dim;
          best_split_type = boosted_trees::SplitTypeWithDefault_Name(
              boosted_trees::INEQUALITY_DEFAULT_RIGHT);
          best_contrib_for_left = contrib_for_left[0];
          best_contrib_for_right = contrib_for_right[0];
        }

        // From right to left, default left.
        CalculateWeightsAndGains((total_left_grad + default_grad) * identity,
                                 (total_left_hess + default_hess) * identity,
                                 l1, l2, &contrib_for_left, &gain_for_left);
        CalculateWeightsAndGains(
            (total_grad - default_grad - total_left_grad) * identity,
            (total_hess - default_hess - total_left_hess) * identity, l1, l2,
            &contrib_for_right, &gain_for_right);
        if (GainIsLarger(gain_for_left + gain_for_right, best_gain)) {
          best_gain = gain_for_left + gain_for_right;
          best_bucket = bucket_id;
          best_f_dim = feature_dim;
          best_split_type = boosted_trees::SplitTypeWithDefault_Name(
              boosted_trees::INEQUALITY_DEFAULT_LEFT);
          best_contrib_for_left = contrib_for_left[0];
          best_contrib_for_right = contrib_for_right[0];
        }
      }  // for bucket_id
    }    // for feature_dim
    if (best_gain != std::numeric_limits<float>::lowest()) {
      output_node_ids->push_back(node_id);
      // Remove the parent gain.
      output_gains->push_back(best_gain - parent_gain);
      output_feature_dimensions->push_back(best_f_dim);
      output_split_types->push_back(best_split_type);
      output_thresholds->push_back(best_bucket);
      output_left_node_contribs->push_back(best_contrib_for_left);
      output_right_node_contribs->push_back(best_contrib_for_right);
    }
  }