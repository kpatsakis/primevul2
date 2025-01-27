Status GetAxisForPackAndUnpack(InferenceContext* c, int32_t rank_after_pack,
                               int32* axis) {
  TF_RETURN_IF_ERROR(c->GetAttr("axis", axis));
  if (*axis < -1 * rank_after_pack || *axis >= rank_after_pack) {
    return errors::InvalidArgument("Invalid axis: ", *axis, "; must be in [",
                                   -1 * rank_after_pack, ",", rank_after_pack,
                                   ")");
  }
  if (*axis < 0) *axis = (rank_after_pack + *axis);
  return Status::OK();
}