inline const int* TensorProtoData<Eigen::half>(const TensorProto& t) {
  return t.half_val().data();
}