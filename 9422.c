void LSTMBlockCellBpropWithEigen(
    const LSTMBlockCell& cell, OpKernelContext* ctx, const Device& d,
    bool use_peephole, typename TTypes<T>::ConstMatrix x,
    typename TTypes<T>::ConstMatrix cs_prev,
    typename TTypes<T>::ConstMatrix h_prev, typename TTypes<T>::ConstMatrix w,
    typename TTypes<T>::ConstVec wci, typename TTypes<T>::ConstVec wcf,
    typename TTypes<T>::ConstVec wco, typename TTypes<T>::ConstVec b,
    typename TTypes<T>::ConstMatrix i, typename TTypes<T>::ConstMatrix cs,
    typename TTypes<T>::ConstMatrix f, typename TTypes<T>::ConstMatrix o,
    typename TTypes<T>::ConstMatrix ci, typename TTypes<T>::ConstMatrix co,
    typename TTypes<T>::ConstMatrix cs_grad,
    typename TTypes<T>::ConstMatrix h_grad, typename TTypes<T>::Matrix do_,
    typename TTypes<T>::Matrix dcs, typename TTypes<T>::Matrix dci,
    typename TTypes<T>::Matrix df, typename TTypes<T>::Matrix di,
    typename TTypes<T>::Matrix dgates, typename TTypes<T>::Matrix cs_prev_grad,
    typename TTypes<T>::Vec wci_grad, typename TTypes<T>::Vec wcf_grad,
    typename TTypes<T>::Vec wco_grad) {
  // do[t] = sigm'(o[t]) .* dh[t] .* co[t]
  do_.device(d) = o * (o.constant(T(1)) - o) * h_grad * co;

  // dcs[t] += tanh'(cs[t]) .* dh[t] .* o[t] + dcs[t + 1] .* f[t + 1]
  dcs.device(d) = (co.constant(T(1)) - co * co) * h_grad * o + cs_grad;

  Eigen::array<Eigen::DenseIndex, 2> p_shape({1, cell.cell_size()});
  Eigen::array<Eigen::DenseIndex, 2> p_broadcast_shape({cell.batch_size(), 1});
  if (use_peephole) {
    dcs.device(d) =
        dcs + do_ * wco.reshape(p_shape).broadcast(p_broadcast_shape);
  }

  // dci[t] = tanh'(ci[t]) dcs[t] i[t]
  dci.device(d) = (ci.constant(T(1)) - ci * ci) * dcs * i;

  // df[t] = sigm'(f[t]) dcs[t] cs[t - 1]
  df.device(d) = f * (f.constant(T(1)) - f) * dcs * cs_prev;

  // di[t] = sigm'(i[t]) dcs[t] ci[t]
  di.device(d) = i * (i.constant(T(1)) - i) * dcs * ci;

  dgates.slice(cell.gates_i_offsets(), cell.cell_extents()).device(d) = di;
  dgates.slice(cell.gates_c_offsets(gate_layout), cell.cell_extents())
      .device(d) = dci;
  dgates.slice(cell.gates_f_offsets(gate_layout), cell.cell_extents())
      .device(d) = df;
  dgates.slice(cell.gates_o_offsets(), cell.cell_extents()).device(d) = do_;

  cs_prev_grad.device(d) = dcs * f;
  if (use_peephole) {
    cs_prev_grad.device(d) =
        cs_prev_grad + di * wci.reshape(p_shape).broadcast(p_broadcast_shape) +
        df * wcf.reshape(p_shape).broadcast(p_broadcast_shape);
    wci_grad.device(d) = (di * cs_prev).sum(Eigen::array<int, 1>({0}));
    wcf_grad.device(d) = (df * cs_prev).sum(Eigen::array<int, 1>({0}));
    wco_grad.device(d) = (do_ * cs).sum(Eigen::array<int, 1>({0}));
  }
}