void Document::SetShadowCascadeOrder(ShadowCascadeOrder order) {
  DCHECK_NE(order, ShadowCascadeOrder::kShadowCascadeNone);

  if (order == shadow_cascade_order_)
    return;

  if (order == ShadowCascadeOrder::kShadowCascadeV0) {
    may_contain_v0_shadow_ = true;
    if (shadow_cascade_order_ == ShadowCascadeOrder::kShadowCascadeV1)
      UseCounter::Count(*this, WebFeature::kMixedShadowRootV0AndV1);
  }

  if (shadow_cascade_order_ == ShadowCascadeOrder::kShadowCascadeV0 &&
      order == ShadowCascadeOrder::kShadowCascadeV1) {
    SetNeedsStyleRecalc(
        kSubtreeStyleChange,
        StyleChangeReasonForTracing::Create(StyleChangeReason::kShadow));
    UseCounter::Count(*this, WebFeature::kMixedShadowRootV0AndV1);
  }

  if (order > shadow_cascade_order_)
    shadow_cascade_order_ = order;
}
