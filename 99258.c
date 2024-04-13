static GF_Err gf_isom_dump_srt_track(GF_ISOFile *the_file, u32 track, FILE *dump)
{
	u32 i, j, k, count, di, len, ts, cur_frame;
	u64 start, end;
	GF_Tx3gSampleEntryBox *txtd;
	GF_BitStream *bs;
	char szDur[100];

	GF_TrackBox *trak = gf_isom_get_track_from_file(the_file, track);
	if (!trak) return GF_BAD_PARAM;
	switch (trak->Media->handler->handlerType) {
	case GF_ISOM_MEDIA_TEXT:
	case GF_ISOM_MEDIA_SUBT:
		break;
	default:
		return GF_BAD_PARAM;
	}

	ts = trak->Media->mediaHeader->timeScale;
	cur_frame = 0;
	end = 0;

	count = gf_isom_get_sample_count(the_file, track);
	for (i=0; i<count; i++) {
		GF_TextSample *txt;
		GF_ISOSample *s = gf_isom_get_sample(the_file, track, i+1, &di);
		if (!s) continue;

		start = s->DTS;
		if (s->dataLength==2) {
			gf_isom_sample_del(&s);
			continue;
		}
		if (i+1<count) {
			GF_ISOSample *next = gf_isom_get_sample_info(the_file, track, i+2, NULL, NULL);
			if (next) {
				end = next->DTS;
				gf_isom_sample_del(&next);
			}
		} else {
			end = gf_isom_get_media_duration(the_file, track) ;
		}
		cur_frame++;
		fprintf(dump, "%d\n", cur_frame);
		tx3g_format_time(start, ts, szDur, GF_TRUE);
		fprintf(dump, "%s --> ", szDur);
		tx3g_format_time(end, ts, szDur, GF_TRUE);
		fprintf(dump, "%s\n", szDur);

		bs = gf_bs_new(s->data, s->dataLength, GF_BITSTREAM_READ);
		txt = gf_isom_parse_texte_sample(bs);
		gf_bs_del(bs);

		txtd = (GF_Tx3gSampleEntryBox *)gf_list_get(trak->Media->information->sampleTable->SampleDescription->other_boxes, di-1);

		if (!txt->len) {
			fprintf(dump, "\n");
		} else {
			u32 styles, char_num, new_styles, color, new_color;
			u16 utf16Line[10000];

			/*UTF16*/
			if ((txt->len>2) && ((unsigned char) txt->text[0] == (unsigned char) 0xFE) && ((unsigned char) txt->text[1] == (unsigned char) 0xFF)) {
				memcpy(utf16Line, txt->text+2, sizeof(char)*txt->len);
				( ((char *)utf16Line)[txt->len] ) = 0;
				len = txt->len;
			} else {
				u8 *str = (u8 *) (txt->text);
				size_t res = gf_utf8_mbstowcs(utf16Line, 10000, (const char **) &str);
				if (res==(size_t)-1) return GF_NON_COMPLIANT_BITSTREAM;
				len = (u32) res;
				utf16Line[len] = 0;
			}
			char_num = 0;
			styles = 0;
			new_styles = txtd->default_style.style_flags;
			color = new_color = txtd->default_style.text_color;

			for (j=0; j<len; j++) {
				Bool is_new_line;

				if (txt->styles) {
					new_styles = txtd->default_style.style_flags;
					new_color = txtd->default_style.text_color;
					for (k=0; k<txt->styles->entry_count; k++) {
						if (txt->styles->styles[k].startCharOffset>char_num) continue;
						if (txt->styles->styles[k].endCharOffset<char_num+1) continue;

						if (txt->styles->styles[k].style_flags & (GF_TXT_STYLE_ITALIC | GF_TXT_STYLE_BOLD | GF_TXT_STYLE_UNDERLINED)) {
							new_styles = txt->styles->styles[k].style_flags;
							new_color = txt->styles->styles[k].text_color;
							break;
						}
					}
				}
				if (new_styles != styles) {
					if ((new_styles & GF_TXT_STYLE_BOLD) && !(styles & GF_TXT_STYLE_BOLD)) fprintf(dump, "<b>");
					if ((new_styles & GF_TXT_STYLE_ITALIC) && !(styles & GF_TXT_STYLE_ITALIC)) fprintf(dump, "<i>");
					if ((new_styles & GF_TXT_STYLE_UNDERLINED) && !(styles & GF_TXT_STYLE_UNDERLINED)) fprintf(dump, "<u>");

					if ((styles & GF_TXT_STYLE_UNDERLINED) && !(new_styles & GF_TXT_STYLE_UNDERLINED)) fprintf(dump, "</u>");
					if ((styles & GF_TXT_STYLE_ITALIC) && !(new_styles & GF_TXT_STYLE_ITALIC)) fprintf(dump, "</i>");
					if ((styles & GF_TXT_STYLE_BOLD) && !(new_styles & GF_TXT_STYLE_BOLD)) fprintf(dump, "</b>");

					styles = new_styles;
				}
				if (new_color != color) {
					if (new_color ==txtd->default_style.text_color) {
						fprintf(dump, "</font>");
					} else {
						fprintf(dump, "<font color=\"%s\">", gf_color_get_name(new_color) );
					}
					color = new_color;
				}

				/*not sure if styles must be reseted at line breaks in srt...*/
				is_new_line = GF_FALSE;
				if ((utf16Line[j]=='\n') || (utf16Line[j]=='\r') ) {
					if ((utf16Line[j]=='\r') && (utf16Line[j+1]=='\n')) j++;
					fprintf(dump, "\n");
					is_new_line = GF_TRUE;
				}

				if (!is_new_line) {
					size_t sl;
					char szChar[30];
					s16 swT[2], *swz;
					swT[0] = utf16Line[j];
					swT[1] = 0;
					swz= (s16 *)swT;
					sl = gf_utf8_wcstombs(szChar, 30, (const unsigned short **) &swz);
					if (sl == (size_t)-1) sl=0;
					szChar[(u32) sl]=0;
					fprintf(dump, "%s", szChar);
				}
				char_num++;
			}
			new_styles = 0;
			if (new_styles != styles) {
				if (styles & GF_TXT_STYLE_UNDERLINED) fprintf(dump, "</u>");
				if (styles & GF_TXT_STYLE_ITALIC) fprintf(dump, "</i>");
				if (styles & GF_TXT_STYLE_BOLD) fprintf(dump, "</b>");

			}

			if (color != txtd->default_style.text_color) {
				fprintf(dump, "</font>");
			}
			fprintf(dump, "\n");
		}
		gf_isom_sample_del(&s);
		gf_isom_delete_text_sample(txt);
		fprintf(dump, "\n");
		gf_set_progress("SRT Extract", i, count);
	}
	if (count) gf_set_progress("SRT Extract", i, count);
	return GF_OK;
}
