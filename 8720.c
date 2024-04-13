std::vector<int64_t> GetFlatInt64(const Tensor& t) {
  std::vector<int64_t> output(t.shape().num_elements());
  if (t.shape().num_elements() > 0) {
    auto eigen_vec = t.flat<InputType>();
    std::copy_n(&eigen_vec(0), output.size(), output.begin());
  }
  return output;
}