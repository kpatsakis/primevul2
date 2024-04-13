GF_Err audio_sample_entry_AddBox(GF_Box *s, GF_Box *a)
{
	GF_MPEGAudioSampleEntryBox *ptr = (GF_MPEGAudioSampleEntryBox *)s;
	switch (a->type) {
	case GF_ISOM_BOX_TYPE_ESDS:
		if (ptr->esd) ERROR_ON_DUPLICATED_BOX(a, ptr)
			ptr->esd = (GF_ESDBox *)a;
		break;
	case GF_ISOM_BOX_TYPE_SINF:
		gf_list_add(ptr->protections, a);
		break;
	case GF_ISOM_BOX_TYPE_DAMR:
	case GF_ISOM_BOX_TYPE_DEVC:
	case GF_ISOM_BOX_TYPE_DQCP:
	case GF_ISOM_BOX_TYPE_DSMV:
		ptr->cfg_3gpp = (GF_3GPPConfigBox *) a;
		/*for 3GP config, remember sample entry type in config*/
		ptr->cfg_3gpp->cfg.type = ptr->type;
		break;

	case GF_ISOM_BOX_TYPE_DAC3:
		ptr->cfg_ac3 = (GF_AC3ConfigBox *) a;
		break;
	case GF_ISOM_BOX_TYPE_DEC3:
		ptr->cfg_ac3 = (GF_AC3ConfigBox *) a;
		break;
	case GF_ISOM_BOX_TYPE_MHA1:
	case GF_ISOM_BOX_TYPE_MHA2:
	case GF_ISOM_BOX_TYPE_MHM1:
	case GF_ISOM_BOX_TYPE_MHM2:
		ptr->cfg_mha = (GF_MHAConfigBox *) a;
		break;

	case GF_ISOM_BOX_TYPE_UNKNOWN:
		/*HACK for QT files: get the esds box from the track*/
		if (s->type == GF_ISOM_BOX_TYPE_MP4A) {
			GF_UnknownBox *wave = (GF_UnknownBox *)a;

			if (ptr->esd) ERROR_ON_DUPLICATED_BOX(a, ptr)

 			if ((wave->original_4cc == GF_QT_BOX_TYPE_WAVE) && gf_list_count(wave->other_boxes)) {
 				u32 i;
                for (i =0; i<gf_list_count(wave->other_boxes); i++) {
                    GF_Box *inner_box = (GF_Box *)gf_list_get(wave->other_boxes, i);
                    if (inner_box->type == GF_ISOM_BOX_TYPE_ESDS) {
                        ptr->esd = (GF_ESDBox *)inner_box;
                    }
                }
                return gf_isom_box_add_default(s, a);
            }

            if (wave->data != NULL) {
                u32 offset = 0;
                while (offset + 5 < wave->dataSize && (wave->data[offset + 4] != 'e') && (wave->data[offset + 5] != 's')) {
                    offset++;
                }
                if (offset + 5 < wave->dataSize) {
                    GF_Box *a;
                    GF_Err e;
                    GF_BitStream *bs = gf_bs_new(wave->data + offset, wave->dataSize - offset, GF_BITSTREAM_READ);
                    e = gf_isom_box_parse(&a, bs);
                    gf_bs_del(bs);
                    if (e) return e;
                    ptr->esd = (GF_ESDBox *)a;
                    gf_isom_box_add_for_dump_mode((GF_Box *)ptr, a);
					gf_isom_box_del(a);
					return GF_OK;
                }
				GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[iso file] Cannot process box %s!\n", gf_4cc_to_str(wave->original_4cc)));
			}
		}
		return gf_isom_box_add_default(s, a);
	default:
		return gf_isom_box_add_default(s, a);
	}
	return GF_OK;
}
