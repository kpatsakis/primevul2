Delete(struct rx_call *call, afs_int32 aid, afs_int32 *cid)
{
    afs_int32 code;
    struct ubik_trans *tt;
    struct prentry tentry;
    afs_int32 loc, nptr;
    int count;

    code = Initdb();
    if (code)
	return code;
    if (code != PRSUCCESS)
	return code;
    if (aid == SYSADMINID || aid == ANYUSERID || aid == AUTHUSERID
	|| aid == ANONYMOUSID)
	return PRPERM;
    code = ubik_BeginTrans(dbase, UBIK_WRITETRANS, &tt);
    if (code)
	return code;
    code = ubik_SetLock(tt, 1, 1, LOCKWRITE);
    if (code)
	ABORT_WITH(tt, code);
    code = read_DbHeader(tt);
    if (code)
	ABORT_WITH(tt, code);

    code = WhoIsThis(call, tt, cid);
    if (code)
	ABORT_WITH(tt, PRPERM);

    /* Read in entry to be deleted */
    loc = FindByID(tt, aid);
    if (loc == 0)
	ABORT_WITH(tt, PRNOENT);
    code = pr_ReadEntry(tt, 0, loc, &tentry);
    if (code)
	ABORT_WITH(tt, PRDBFAIL);

    /* Do some access checking */
    if (tentry.owner != *cid && !IsAMemberOf(tt, *cid, SYSADMINID)
	&& !IsAMemberOf(tt, *cid, tentry.owner) && !pr_noAuth)
	ABORT_WITH(tt, PRPERM);

    /* Delete each continuation block as a separate transaction so that no one
     * transaction become to large to complete. */
    nptr = tentry.next;
    while (nptr != 0) {
	struct contentry centry;
	int i;

	code = pr_ReadCoEntry(tt, 0, nptr, &centry);
	if (code != 0)
	    ABORT_WITH(tt, PRDBFAIL);
	for (i = 0; i < COSIZE; i++) {
	    if (centry.entries[i] == PRBADID)
		continue;
	    if (centry.entries[i] == 0)
		break;
#if defined(SUPERGROUPS)
	    if (aid < 0 && centry.entries[i] < 0)	/* Supergroup */
		code = RemoveFromSGEntry(tt, aid, centry.entries[i]);
	    else
#endif
		code = RemoveFromEntry(tt, aid, centry.entries[i]);
	    if (code)
		ABORT_WITH(tt, code);
	    tentry.count--;	/* maintain count */
#ifndef AFS_PTHREAD_ENV
	    if ((i & 3) == 0)
		IOMGR_Poll();
#endif
	}
	tentry.next = centry.next;	/* thread out this block */
	code = FreeBlock(tt, nptr);	/* free continuation block */
	if (code)
	    ABORT_WITH(tt, code);
	code = pr_WriteEntry(tt, 0, loc, &tentry);	/* update main entry */
	if (code)
	    ABORT_WITH(tt, code);

	/* end this trans and start a new one */
	code = ubik_EndTrans(tt);
	if (code)
	    return code;
#ifndef AFS_PTHREAD_ENV
	IOMGR_Poll();		/* just to keep the connection alive */
#endif
	code = ubik_BeginTrans(dbase, UBIK_WRITETRANS, &tt);
	if (code)
	    return code;
	code = ubik_SetLock(tt, 1, 1, LOCKWRITE);
	if (code)
	    ABORT_WITH(tt, code);

	/* re-read entry to get consistent uptodate info */
	loc = FindByID(tt, aid);
	if (loc == 0)
	    ABORT_WITH(tt, PRNOENT);
	code = pr_ReadEntry(tt, 0, loc, &tentry);
	if (code)
	    ABORT_WITH(tt, PRDBFAIL);

	nptr = tentry.next;
    }

#if defined(SUPERGROUPS)
    /* Delete each continuation block as a separate transaction
     * so that no one transaction become too large to complete. */
    {
	struct prentryg *tentryg = (struct prentryg *)&tentry;
	nptr = tentryg->nextsg;
	while (nptr != 0) {
	    struct contentry centry;
	    int i;

	    code = pr_ReadCoEntry(tt, 0, nptr, &centry);
	    if (code != 0)
		ABORT_WITH(tt, PRDBFAIL);
	    for (i = 0; i < COSIZE; i++) {
		if (centry.entries[i] == PRBADID)
		    continue;
		if (centry.entries[i] == 0)
		    break;
		code = RemoveFromEntry(tt, aid, centry.entries[i]);
		if (code)
		    ABORT_WITH(tt, code);
		tentryg->countsg--;	/* maintain count */
#ifndef AFS_PTHREAD_ENV
		if ((i & 3) == 0)
		    IOMGR_Poll();
#endif
	    }
	    tentryg->nextsg = centry.next;	/* thread out this block */
	    code = FreeBlock(tt, nptr);	/* free continuation block */
	    if (code)
		ABORT_WITH(tt, code);
	    code = pr_WriteEntry(tt, 0, loc, &tentry);	/* update main entry */
	    if (code)
		ABORT_WITH(tt, code);

	    /* end this trans and start a new one */
	    code = ubik_EndTrans(tt);
	    if (code)
		return code;
#ifndef AFS_PTHREAD_ENV
	    IOMGR_Poll();	/* just to keep the connection alive */
#endif

	    code = ubik_BeginTrans(dbase, UBIK_WRITETRANS, &tt);
	    if (code)
		return code;
	    code = ubik_SetLock(tt, 1, 1, LOCKWRITE);
	    if (code)
		ABORT_WITH(tt, code);

	    /* re-read entry to get consistent uptodate info */
	    loc = FindByID(tt, aid);
	    if (loc == 0)
		ABORT_WITH(tt, PRNOENT);
	    code = pr_ReadEntry(tt, 0, loc, &tentry);
	    if (code)
		ABORT_WITH(tt, PRDBFAIL);

	    nptr = tentryg->nextsg;
	}
    }

#endif /* SUPERGROUPS */

    /* Then move the owned chain, except possibly ourself to the orphan list.
     * Because this list can be very long and so exceed the size of a ubik
     * transaction, we start a new transaction every 50 entries. */
    count = 0;
    nptr = tentry.owned;
    while (nptr != 0) {
	struct prentry nentry;

	code = pr_ReadEntry(tt, 0, nptr, &nentry);
	if (code)
	    ABORT_WITH(tt, PRDBFAIL);
	nptr = tentry.owned = nentry.nextOwned;	/* thread out */

	if (nentry.id != tentry.id) {	/* don't add us to orphan chain! */
	    code = AddToOrphan(tt, nentry.id);
	    if (code)
		ABORT_WITH(tt, code);
	    count++;
#ifndef AFS_PTHREAD_ENV
	    if ((count & 3) == 0)
		IOMGR_Poll();
#endif
	}
	if (count < 50)
	    continue;
	code = pr_WriteEntry(tt, 0, loc, &tentry);	/* update main entry */
	if (code)
	    ABORT_WITH(tt, code);

	/* end this trans and start a new one */
	code = ubik_EndTrans(tt);
	if (code)
	    return code;
#ifndef AFS_PTHREAD_ENV
	IOMGR_Poll();		/* just to keep the connection alive */
#endif
	code = ubik_BeginTrans(dbase, UBIK_WRITETRANS, &tt);
	if (code)
	    return code;
	code = ubik_SetLock(tt, 1, 1, LOCKWRITE);
	if (code)
	    ABORT_WITH(tt, code);

	/* re-read entry to get consistent uptodate info */
	loc = FindByID(tt, aid);
	if (loc == 0)
	    ABORT_WITH(tt, PRNOENT);
	code = pr_ReadEntry(tt, 0, loc, &tentry);
	if (code)
	    ABORT_WITH(tt, PRDBFAIL);

	nptr = tentry.owned;
    }

    /* now do what's left of the deletion stuff */
    code = DeleteEntry(tt, &tentry, loc);
    if (code != PRSUCCESS)
	ABORT_WITH(tt, code);

    code = ubik_EndTrans(tt);
    if (code)
	return code;
    return PRSUCCESS;
}
