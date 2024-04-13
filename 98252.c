static void __match_proto__(task_func_t)
vbf_fetch_thread(struct worker *wrk, void *priv)
{
	struct busyobj *bo;
	enum fetch_step stp;

	CHECK_OBJ_NOTNULL(wrk, WORKER_MAGIC);
	CAST_OBJ_NOTNULL(bo, priv, BUSYOBJ_MAGIC);
	CHECK_OBJ_NOTNULL(bo->req, REQ_MAGIC);
	CHECK_OBJ_NOTNULL(bo->fetch_objcore, OBJCORE_MAGIC);

	THR_SetBusyobj(bo);
	stp = F_STP_MKBEREQ;
	assert(isnan(bo->t_first));
	assert(isnan(bo->t_prev));
	VSLb_ts_busyobj(bo, "Start", W_TIM_real(wrk));

	bo->wrk = wrk;
	wrk->vsl = bo->vsl;

#if 0
	if (bo->stale_oc != NULL) {
		CHECK_OBJ_NOTNULL(bo->stale_oc, OBJCORE_MAGIC);
		/* We don't want the oc/stevedore ops in fetching thread */
		if (!ObjCheckFlag(wrk, bo->stale_oc, OF_IMSCAND))
			(void)HSH_DerefObjCore(wrk, &bo->stale_oc, 0);
	}
#endif

	while (stp != F_STP_DONE) {
		CHECK_OBJ_NOTNULL(bo, BUSYOBJ_MAGIC);
		assert(bo->fetch_objcore->boc->refcount >= 1);
		switch (stp) {
#define FETCH_STEP(l, U, arg)						\
		case F_STP_##U:						\
			stp = vbf_stp_##l arg;				\
			break;
#include "tbl/steps.h"
		default:
			WRONG("Illegal fetch_step");
		}
	}

	assert(bo->director_state == DIR_S_NULL);

	http_Teardown(bo->bereq);
	http_Teardown(bo->beresp);

	if (bo->fetch_objcore->boc->state == BOS_FINISHED) {
		AZ(bo->fetch_objcore->flags & OC_F_FAILED);
		VSLb(bo->vsl, SLT_Length, "%ju",
		    (uintmax_t)ObjGetLen(bo->wrk, bo->fetch_objcore));
	}

	if (bo->stale_oc != NULL)
		(void)HSH_DerefObjCore(wrk, &bo->stale_oc, 0);

	wrk->vsl = NULL;
	HSH_DerefBoc(wrk, bo->fetch_objcore);
	SES_Rel(bo->sp);
	VBO_ReleaseBusyObj(wrk, &bo);
	THR_SetBusyobj(NULL);
}
