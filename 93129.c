sg_fill_request_table(Sg_fd *sfp, sg_req_info_t *rinfo)
{
	Sg_request *srp;
	int val;
	unsigned int ms;

	val = 0;
 	list_for_each_entry(srp, &sfp->rq_list, entry) {
 		if (val > SG_MAX_QUEUE)
 			break;
 		rinfo[val].req_state = srp->done + 1;
 		rinfo[val].problem =
 			srp->header.masked_status &
			srp->header.host_status &
			srp->header.driver_status;
		if (srp->done)
			rinfo[val].duration =
				srp->header.duration;
		else {
			ms = jiffies_to_msecs(jiffies);
			rinfo[val].duration =
				(ms > srp->header.duration) ?
				(ms - srp->header.duration) : 0;
		}
		rinfo[val].orphan = srp->orphan;
		rinfo[val].sg_io_owned = srp->sg_io_owned;
		rinfo[val].pack_id = srp->header.pack_id;
		rinfo[val].usr_ptr = srp->header.usr_ptr;
		val++;
	}
}
