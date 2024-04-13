fiber_init(mrb_state *mrb, mrb_value self)
{
  static const struct mrb_context mrb_context_zero = { 0 };
  struct RFiber *f = fiber_ptr(self);
  struct mrb_context *c;
  struct RProc *p;
  mrb_callinfo *ci;
  mrb_value blk;
  size_t slen;

  mrb_get_args(mrb, "&!", &blk);

  if (f->cxt) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "cannot initialize twice");
  }
  p = mrb_proc_ptr(blk);
  if (MRB_PROC_CFUNC_P(p)) {
    mrb_raise(mrb, E_FIBER_ERROR, "tried to create Fiber from C defined method");
  }

  c = (struct mrb_context*)mrb_malloc(mrb, sizeof(struct mrb_context));
  *c = mrb_context_zero;
  f->cxt = c;

  /* initialize VM stack */
  slen = FIBER_STACK_INIT_SIZE;
  if (p->body.irep->nregs > slen) {
    slen += p->body.irep->nregs;
  }
  c->stbase = (mrb_value *)mrb_malloc(mrb, slen*sizeof(mrb_value));
  c->stend = c->stbase + slen;

  {
    mrb_value *p = c->stbase;
    mrb_value *pend = c->stend;

    while (p < pend) {
      SET_NIL_VALUE(*p);
      p++;
    }
  }

  /* copy receiver from a block */
  c->stbase[0] = mrb->c->ci->stack[0];

  /* initialize callinfo stack */
  c->cibase = (mrb_callinfo *)mrb_calloc(mrb, FIBER_CI_INIT_SIZE, sizeof(mrb_callinfo));
  c->ciend = c->cibase + FIBER_CI_INIT_SIZE;
  c->ci = c->cibase;

  /* adjust return callinfo */
  ci = c->ci;
  mrb_vm_ci_target_class_set(ci, MRB_PROC_TARGET_CLASS(p));
  mrb_vm_ci_proc_set(ci, p);
  mrb_field_write_barrier(mrb, (struct RBasic*)mrb_obj_ptr(self), (struct RBasic*)p);
  ci->stack = c->stbase;
  ci[1] = ci[0];
  c->ci++;                      /* push dummy callinfo */

  c->fib = f;
  c->status = MRB_FIBER_CREATED;

  return self;
}