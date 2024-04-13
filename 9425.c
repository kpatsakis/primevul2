void LSTMBlockCellFpropWithEigen(
    const LSTMBlockCell& cell, OpKernelContext* ctx, const CPUDevice& d,
    const float forget_bias, const float cell_clip, bool use_peephole,
    typename TTypes<T>::ConstMatrix x, typename TTypes<T>::ConstMatrix cs_prev,
    typename TTypes<T>::ConstMatrix h_prev, typename TTypes<T>::ConstMatrix w,
    typename TTypes<T>::ConstVec wci, typename TTypes<T>::ConstVec wcf,
    typename TTypes<T>::ConstVec wco, typename TTypes<T>::ConstVec b,
    typename TTypes<T>::Matrix xh, typename TTypes<T>::Matrix i,
    typename TTypes<T>::Matrix cs, typename TTypes<T>::Matrix f,
    typename TTypes<T>::Matrix o, typename TTypes<T>::Matrix ci,
    typename TTypes<T>::Matrix co, typename TTypes<T>::Matrix gates,
    typename TTypes<T>::Matrix h) {
  // Concat xh = [x, h].
  xh.slice(cell.xh_x_offsets(), cell.xh_x_extents()).device(d) = x;
  xh.slice(cell.xh_h_offsets(), cell.xh_h_extents()).device(d) = h_prev;

  // states1 = xh * w + b
  typename TTypes<T>::ConstMatrix const_xh(xh.data(), xh.dimensions());
  TensorBlasGemm<CPUDevice, T, false /* USE_CUBLAS */>::compute(
      ctx, d, false, false, typename gemm_compute_type<T>::type(1.f), const_xh,
      w, typename gemm_compute_type<T>::type(0.f), gates);
  Eigen::array<Eigen::DenseIndex, 2> b_shape({1, b.dimensions()[0]});
  Eigen::array<Eigen::DenseIndex, 2> broadcast_shape({cell.batch_size(), 1});
  gates.device(d) += b.reshape(b_shape).broadcast(broadcast_shape);

  Eigen::array<Eigen::DenseIndex, 2> p_shape({1, cell.cell_size()});
  Eigen::array<Eigen::DenseIndex, 2> p_broadcast_shape({cell.batch_size(), 1});

  // Input gate.
  if (use_peephole) {
    auto i_peep = cs_prev * wci.reshape(p_shape).broadcast(p_broadcast_shape);
    i.device(d) =
        (gates.slice(cell.gates_i_offsets(), cell.cell_extents()) + i_peep)
            .sigmoid();
  } else {
    i.device(d) =
        gates.slice(cell.gates_i_offsets(), cell.cell_extents()).sigmoid();
  }

  // Cell input.
  ci.device(d) =
      gates.slice(cell.gates_c_offsets(gate_layout), cell.cell_extents())
          .tanh();

  // Forget gate (w/ bias).
  if (use_peephole) {
    auto f_peep = cs_prev * wcf.reshape(p_shape).broadcast(p_broadcast_shape);
    f.device(d) =
        (gates.slice(cell.gates_f_offsets(gate_layout), cell.cell_extents()) +
         f.constant(T(forget_bias)) + f_peep)
            .sigmoid();
  } else {
    f.device(d) =
        (gates.slice(cell.gates_f_offsets(gate_layout), cell.cell_extents()) +
         f.constant(T(forget_bias)))
            .sigmoid();
  }

  // cs = ci .* i + f .* cs_prev
  cs.device(d) = i * ci + f * cs_prev;

  if (cell_clip > 0.0f) {
    cs.device(d) =
        cs.binaryExpr(cs.constant(T(cell_clip)), Eigen::scalar_clip_op<T>());
  }

  // co = tanh(cs)
  co.device(d) = cs.tanh();

  // Output gate.
  if (use_peephole) {
    auto o_peep = cs * wco.reshape(p_shape).broadcast(p_broadcast_shape);
    o.device(d) =
        (gates.slice(cell.gates_o_offsets(), cell.cell_extents()) + o_peep)
            .sigmoid();
  } else {
    o.device(d) =
        gates.slice(cell.gates_o_offsets(), cell.cell_extents()).sigmoid();
  }

  // h = o .* co
  h.device(d) = o * co;
}