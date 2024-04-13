static exporter_ipfix_domain_t *GetExporter(FlowSource_t *fs, ipfix_header_t *ipfix_header) {
#define IP_STRING_LEN   40
char ipstr[IP_STRING_LEN];
exporter_ipfix_domain_t **e = (exporter_ipfix_domain_t **)&(fs->exporter_data);
uint32_t ObservationDomain = ntohl(ipfix_header->ObservationDomain);

	while ( *e ) {
		if ( (*e)->info.id == ObservationDomain && (*e)->info.version == 10 && 
			 (*e)->info.ip.V6[0] == fs->ip.V6[0] && (*e)->info.ip.V6[1] == fs->ip.V6[1]) 
			return *e;
		e = &((*e)->next);
	}

	if ( fs->sa_family == AF_INET ) {
		uint32_t _ip = htonl(fs->ip.V4);
		inet_ntop(AF_INET, &_ip, ipstr, sizeof(ipstr));
	} else if ( fs->sa_family == AF_INET6 ) {
		uint64_t _ip[2];
		_ip[0] = htonll(fs->ip.V6[0]);
		_ip[1] = htonll(fs->ip.V6[1]);
		inet_ntop(AF_INET6, &_ip, ipstr, sizeof(ipstr));
	} else {
		strncpy(ipstr, "<unknown>", IP_STRING_LEN);
	}

	*e = (exporter_ipfix_domain_t *)malloc(sizeof(exporter_ipfix_domain_t));
	if ( !(*e)) {
		LogError("Process_ipfix: Panic! malloc() %s line %d: %s", __FILE__, __LINE__, strerror (errno));
		return NULL;
	}
	memset((void *)(*e), 0, sizeof(exporter_ipfix_domain_t));
	(*e)->info.header.type  = ExporterInfoRecordType;
	(*e)->info.header.size  = sizeof(exporter_info_record_t);
	(*e)->info.id 			= ObservationDomain;
	(*e)->info.ip			= fs->ip;
	(*e)->info.sa_family	= fs->sa_family;
	(*e)->info.version 		= 10;
	(*e)->info.sysid	 	= 0;

	(*e)->TemplateRecords 	= 0;
	(*e)->DataRecords 	 	= 0;
	(*e)->sequence_failure 	= 0;
	(*e)->padding_errors 	= 0;
	(*e)->next	 			= NULL;
	(*e)->sampler 			= NULL;

	FlushInfoExporter(fs, &((*e)->info));

	dbg_printf("[%u] New exporter: SysID: %u, Observation domain %u from: %s\n", 
		ObservationDomain, (*e)->info.sysid, ObservationDomain, ipstr);
	LogInfo("Process_ipfix: New exporter: SysID: %u, Observation domain %u from: %s\n", 
		(*e)->info.sysid, ObservationDomain, ipstr);


	return (*e);

} // End of GetExporter
