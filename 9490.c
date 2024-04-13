static void handle_TYPE(ctrl_t *ctrl, char *argument)
{
	char type[24]  = "200 Type set to I.\r\n";
	char unknown[] = "501 Invalid argument to TYPE.\r\n";

	if (!argument)
		argument = "Z";

	switch (argument[0]) {
	case 'A':
		ctrl->type = TYPE_A; /* ASCII */
		break;

	case 'I':
		ctrl->type = TYPE_I; /* IMAGE/BINARY */
		break;

	default:
		send_msg(ctrl->sd, unknown);
		return;
	}

	type[16] = argument[0];
	send_msg(ctrl->sd, type);
}