static noinline int hiddev_ioctl_usage(struct hiddev *hiddev, unsigned int cmd, void __user *user_arg)
{
	struct hid_device *hid = hiddev->hid;
	struct hiddev_report_info rinfo;
	struct hiddev_usage_ref_multi *uref_multi = NULL;
	struct hiddev_usage_ref *uref;
	struct hid_report *report;
	struct hid_field *field;
	int i;

	uref_multi = kmalloc(sizeof(struct hiddev_usage_ref_multi), GFP_KERNEL);
	if (!uref_multi)
		return -ENOMEM;
	uref = &uref_multi->uref;
	if (cmd == HIDIOCGUSAGES || cmd == HIDIOCSUSAGES) {
		if (copy_from_user(uref_multi, user_arg,
				   sizeof(*uref_multi)))
			goto fault;
	} else {
		if (copy_from_user(uref, user_arg, sizeof(*uref)))
			goto fault;
	}

	switch (cmd) {
	case HIDIOCGUCODE:
		rinfo.report_type = uref->report_type;
		rinfo.report_id = uref->report_id;
		if ((report = hiddev_lookup_report(hid, &rinfo)) == NULL)
			goto inval;

		if (uref->field_index >= report->maxfield)
			goto inval;

		field = report->field[uref->field_index];
		if (uref->usage_index >= field->maxusage)
			goto inval;

		uref->usage_code = field->usage[uref->usage_index].hid;

		if (copy_to_user(user_arg, uref, sizeof(*uref)))
			goto fault;

		goto goodreturn;

	default:
		if (cmd != HIDIOCGUSAGE &&
		    cmd != HIDIOCGUSAGES &&
		    uref->report_type == HID_REPORT_TYPE_INPUT)
			goto inval;

		if (uref->report_id == HID_REPORT_ID_UNKNOWN) {
			field = hiddev_lookup_usage(hid, uref);
			if (field == NULL)
				goto inval;
		} else {
			rinfo.report_type = uref->report_type;
			rinfo.report_id = uref->report_id;
			if ((report = hiddev_lookup_report(hid, &rinfo)) == NULL)
				goto inval;

			if (uref->field_index >= report->maxfield)
				goto inval;

			field = report->field[uref->field_index];

			if (cmd == HIDIOCGCOLLECTIONINDEX) {
				if (uref->usage_index >= field->maxusage)
 					goto inval;
 			} else if (uref->usage_index >= field->report_count)
 				goto inval;
 		}
 
		if ((cmd == HIDIOCGUSAGES || cmd == HIDIOCSUSAGES) &&
		    (uref_multi->num_values > HID_MAX_MULTI_USAGES ||
		     uref->usage_index + uref_multi->num_values > field->report_count))
			goto inval;

 		switch (cmd) {
 		case HIDIOCGUSAGE:
 			uref->value = field->value[uref->usage_index];
			if (copy_to_user(user_arg, uref, sizeof(*uref)))
				goto fault;
			goto goodreturn;

		case HIDIOCSUSAGE:
			field->value[uref->usage_index] = uref->value;
			goto goodreturn;

		case HIDIOCGCOLLECTIONINDEX:
			i = field->usage[uref->usage_index].collection_index;
			kfree(uref_multi);
			return i;
		case HIDIOCGUSAGES:
			for (i = 0; i < uref_multi->num_values; i++)
				uref_multi->values[i] =
				    field->value[uref->usage_index + i];
			if (copy_to_user(user_arg, uref_multi,
					 sizeof(*uref_multi)))
				goto fault;
			goto goodreturn;
		case HIDIOCSUSAGES:
			for (i = 0; i < uref_multi->num_values; i++)
				field->value[uref->usage_index + i] =
				    uref_multi->values[i];
			goto goodreturn;
		}

goodreturn:
		kfree(uref_multi);
		return 0;
fault:
		kfree(uref_multi);
		return -EFAULT;
inval:
		kfree(uref_multi);
		return -EINVAL;
	}
}
