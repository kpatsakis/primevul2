static void handle_OPTS(ctrl_t *ctrl, char *arg)
{
	/* OPTS MLST type;size;modify;perm; */
	if (strstr(arg, "MLST")) {
		size_t i = 0;
		char *ptr;
		char buf[42] = "200 MLST OPTS ";
		char facts[10] = { 0 };

		ptr = strtok(arg + 4, " \t;");
		while (ptr && i < sizeof(facts) - 1) {
			if (!strcmp(ptr, "modify") ||
			    !strcmp(ptr, "perm")   ||
			    !strcmp(ptr, "size")   ||
			    !strcmp(ptr, "type")) {
				facts[i++] = ptr[0];
				strlcat(buf, ptr, sizeof(buf));
				strlcat(buf, ";", sizeof(buf));
			}

			ptr = strtok(NULL, ";");
		}
		strlcat(buf, "\r\n", sizeof(buf));

		DBG("New MLSD facts: %s", facts);
		strlcpy(ctrl->facts, facts, sizeof(ctrl->facts));
		send_msg(ctrl->sd, buf);
	} else
		send_msg(ctrl->sd, "200 UTF8 OPTS ON\r\n");
}