std::vector<int64_t> AsInt64(const Tensor* tensor, int64_t num_elements) {
  std::vector<int64_t> ret(num_elements);
  auto data = tensor->vec<T>();
  for (int64_t i = 0; i < num_elements; ++i) {
    ret[i] = data(i);
  }
  return ret;
}