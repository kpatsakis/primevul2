static int samldb_objectclass_trigger(struct samldb_ctx *ac)
{
	struct ldb_context *ldb = ldb_module_get_ctx(ac->module);
	void *skip_allocate_sids = ldb_get_opaque(ldb,
						  "skip_allocate_sids");
	struct ldb_message_element *el, *el2;
	struct dom_sid *sid;
	int ret;

	/* make sure that "sAMAccountType" is not specified */
	el = ldb_msg_find_element(ac->msg, "sAMAccountType");
	if (el != NULL) {
		ldb_set_errstring(ldb,
				  "samldb: sAMAccountType must not be specified!");
		return LDB_ERR_UNWILLING_TO_PERFORM;
	}

	/* Step 1: objectSid assignment */

	/* Don't allow the objectSid to be changed. But beside the RELAX
	 * control we have also to guarantee that it can always be set with
	 * SYSTEM permissions. This is needed for the "samba3sam" backend. */
	sid = samdb_result_dom_sid(ac, ac->msg, "objectSid");
	if ((sid != NULL) && (!dsdb_module_am_system(ac->module)) &&
	    (ldb_request_get_control(ac->req, LDB_CONTROL_RELAX_OID) == NULL)) {
		ldb_set_errstring(ldb,
				  "samldb: objectSid must not be specified!");
		return LDB_ERR_UNWILLING_TO_PERFORM;
	}

	/* but generate a new SID when we do have an add operations */
	if ((sid == NULL) && (ac->req->operation == LDB_ADD) && !skip_allocate_sids) {
		ret = samldb_add_step(ac, samldb_allocate_sid);
		if (ret != LDB_SUCCESS) return ret;
	}

	switch(ac->type) {
	case SAMLDB_TYPE_USER: {
		bool uac_generated = false, uac_add_flags = false;

		/* Step 1.2: Default values */
		ret = dsdb_user_obj_set_defaults(ldb, ac->msg);
		if (ret != LDB_SUCCESS) return ret;

		/* On add operations we might need to generate a
		 * "userAccountControl" (if it isn't specified). */
		el = ldb_msg_find_element(ac->msg, "userAccountControl");
		if ((el == NULL) && (ac->req->operation == LDB_ADD)) {
			ret = samdb_msg_set_uint(ldb, ac->msg, ac->msg,
						 "userAccountControl",
						 UF_NORMAL_ACCOUNT);
			if (ret != LDB_SUCCESS) {
				return ret;
			}
			uac_generated = true;
			uac_add_flags = true;
		}

		el = ldb_msg_find_element(ac->msg, "userAccountControl");
		if (el != NULL) {
			uint32_t user_account_control;
			/* Step 1.3: "userAccountControl" -> "sAMAccountType" mapping */
			user_account_control = ldb_msg_find_attr_as_uint(ac->msg,
									 "userAccountControl",
									 0);
			/*
			 * "userAccountControl" = 0 or missing one of
			 * the types means "UF_NORMAL_ACCOUNT".  See
			 * MS-SAMR 3.1.1.8.10 point 8
			 */
			if ((user_account_control & UF_ACCOUNT_TYPE_MASK) == 0) {
				user_account_control = UF_NORMAL_ACCOUNT | user_account_control;
				uac_generated = true;
			}

			/*
			 * As per MS-SAMR 3.1.1.8.10 these flags have not to be set
			 */
			if ((user_account_control & UF_LOCKOUT) != 0) {
				user_account_control &= ~UF_LOCKOUT;
				uac_generated = true;
			}
			if ((user_account_control & UF_PASSWORD_EXPIRED) != 0) {
				user_account_control &= ~UF_PASSWORD_EXPIRED;
				uac_generated = true;
			}

			ret = samldb_check_user_account_control_rules(ac, NULL,
								      user_account_control, 0);
			if (ret != LDB_SUCCESS) {
				return ret;
			}

			/* Workstation and (read-only) DC objects do need objectclass "computer" */
			if ((samdb_find_attribute(ldb, ac->msg,
						  "objectclass", "computer") == NULL) &&
			    (user_account_control &
			     (UF_SERVER_TRUST_ACCOUNT | UF_WORKSTATION_TRUST_ACCOUNT))) {
				ldb_set_errstring(ldb,
						  "samldb: Requested account type does need objectclass 'computer'!");
				return LDB_ERR_OBJECT_CLASS_VIOLATION;
			}

			/* add "sAMAccountType" attribute */
			ret = dsdb_user_obj_set_account_type(ldb, ac->msg, user_account_control, NULL);
			if (ret != LDB_SUCCESS) {
				return ret;
			}

			/* "isCriticalSystemObject" might be set */
			if (user_account_control &
			    (UF_SERVER_TRUST_ACCOUNT | UF_PARTIAL_SECRETS_ACCOUNT)) {
				ret = ldb_msg_add_string(ac->msg, "isCriticalSystemObject",
							 "TRUE");
				if (ret != LDB_SUCCESS) {
					return ret;
				}
				el2 = ldb_msg_find_element(ac->msg,
							   "isCriticalSystemObject");
				el2->flags = LDB_FLAG_MOD_REPLACE;
			} else if (user_account_control & UF_WORKSTATION_TRUST_ACCOUNT) {
				ret = ldb_msg_add_string(ac->msg, "isCriticalSystemObject",
							 "FALSE");
				if (ret != LDB_SUCCESS) {
					return ret;
				}
				el2 = ldb_msg_find_element(ac->msg,
							   "isCriticalSystemObject");
				el2->flags = LDB_FLAG_MOD_REPLACE;
			}

			/* Step 1.4: "userAccountControl" -> "primaryGroupID" mapping */
			if (!ldb_msg_find_element(ac->msg, "primaryGroupID")) {
				uint32_t rid;

				ret = dsdb_user_obj_set_primary_group_id(ldb, ac->msg, user_account_control, &rid);
				if (ret != LDB_SUCCESS) {
					return ret;
				}
				/*
				 * Older AD deployments don't know about the
				 * RODC group
				 */
				if (rid == DOMAIN_RID_READONLY_DCS) {
					ret = samldb_prim_group_tester(ac, rid);
					if (ret != LDB_SUCCESS) {
						return ret;
					}
				}
			}

			/* Step 1.5: Add additional flags when needed */
			/* Obviously this is done when the "userAccountControl"
			 * has been generated here (tested against Windows
			 * Server) */
			if (uac_generated) {
				if (uac_add_flags) {
					user_account_control |= UF_ACCOUNTDISABLE;
					user_account_control |= UF_PASSWD_NOTREQD;
				}

				ret = samdb_msg_set_uint(ldb, ac->msg, ac->msg,
							 "userAccountControl",
							 user_account_control);
				if (ret != LDB_SUCCESS) {
					return ret;
				}
			}

		}
		break;
	}

	case SAMLDB_TYPE_GROUP: {
		const char *tempstr;

		/* Step 2.2: Default values */
		tempstr = talloc_asprintf(ac->msg, "%d",
					  GTYPE_SECURITY_GLOBAL_GROUP);
		if (tempstr == NULL) return ldb_operr(ldb);
		ret = samdb_find_or_add_attribute(ldb, ac->msg,
			"groupType", tempstr);
		if (ret != LDB_SUCCESS) return ret;

		/* Step 2.3: "groupType" -> "sAMAccountType" */
		el = ldb_msg_find_element(ac->msg, "groupType");
		if (el != NULL) {
			uint32_t group_type, account_type;

			group_type = ldb_msg_find_attr_as_uint(ac->msg,
							       "groupType", 0);

			/* The creation of builtin groups requires the
			 * RELAX control */
			if (group_type == GTYPE_SECURITY_BUILTIN_LOCAL_GROUP) {
				if (ldb_request_get_control(ac->req,
							    LDB_CONTROL_RELAX_OID) == NULL) {
					return LDB_ERR_UNWILLING_TO_PERFORM;
				}
			}

			account_type = ds_gtype2atype(group_type);
			if (account_type == 0) {
				ldb_set_errstring(ldb, "samldb: Unrecognized account type!");
				return LDB_ERR_UNWILLING_TO_PERFORM;
			}
			ret = samdb_msg_add_uint(ldb, ac->msg, ac->msg,
						 "sAMAccountType",
						 account_type);
			if (ret != LDB_SUCCESS) {
				return ret;
			}
			el2 = ldb_msg_find_element(ac->msg, "sAMAccountType");
			el2->flags = LDB_FLAG_MOD_REPLACE;
		}
		break;
	}

	default:
		ldb_asprintf_errstring(ldb,
				"Invalid entry type!");
		return LDB_ERR_OPERATIONS_ERROR;
		break;
	}

	return LDB_SUCCESS;
}
