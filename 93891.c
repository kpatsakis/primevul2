 size_t OpenMP4Source(char *filename, uint32_t traktype, uint32_t traksubtype)  //RAW or within MP4
{
	mp4object *mp4 = (mp4object *)malloc(sizeof(mp4object));
	if (mp4 == NULL) return 0;
 
 	memset(mp4, 0, sizeof(mp4object));
 
	struct stat64 mp4stat;
	stat64(filename, &mp4stat);
	mp4->filesize = mp4stat.st_size;

	if (mp4->filesize < 64) return 0;

 #ifdef _WINDOWS
 	fopen_s(&mp4->mediafp, filename, "rb");
 #else
	mp4->mediafp = fopen(filename, "rb");
#endif

	if (mp4->mediafp)
	{
		uint32_t qttag, qtsize32, skip, type = 0, subtype = 0, num;
		size_t len;
		int32_t nest = 0;
 		uint64_t nestsize[MAX_NEST_LEVEL] = { 0 };
 		uint64_t lastsize = 0, qtsize;
 

 		do
 		{
 			len = fread(&qtsize32, 1, 4, mp4->mediafp);
 			len += fread(&qttag, 1, 4, mp4->mediafp);
			mp4->filepos += len;
			if (len == 8 && mp4->filepos < mp4->filesize)
 			{
 				if (!VALID_FOURCC(qttag))
 				{
					CloseSource((size_t)mp4);
					mp4 = NULL;
					break;
 				}
 
 				qtsize32 = BYTESWAP32(qtsize32);
 
 				if (qtsize32 == 1) // 64-bit Atom
 				{
					len = fread(&qtsize, 1, 8, mp4->mediafp);
					mp4->filepos += len;
 					qtsize = BYTESWAP64(qtsize) - 8;
 				}
 				else
					qtsize = qtsize32;

				nest++;

				if (qtsize < 8) break;
				if (nest >= MAX_NEST_LEVEL) break;

				nestsize[nest] = qtsize;
				lastsize = qtsize;

#if PRINT_MP4_STRUCTURE	

				for (int i = 1; i < nest; i++) printf("    ");
				printf("%c%c%c%c (%lld)\n", (qttag & 0xff), ((qttag >> 8) & 0xff), ((qttag >> 16) & 0xff), ((qttag >> 24) & 0xff), qtsize);
 
 				if (qttag == MAKEID('m', 'd', 'a', 't') ||
 					qttag == MAKEID('f', 't', 'y', 'p') ||
					qttag == MAKEID('u', 'd', 't', 'a') ||
					qttag == MAKEID('f', 'r', 'e', 'e'))
 				{
					LongSeek(mp4, qtsize - 8);
 
 					NESTSIZE(qtsize);
 
					continue;
				}
#else
				if (qttag != MAKEID('m', 'o', 'o', 'v') && //skip over all but these atoms
					qttag != MAKEID('m', 'v', 'h', 'd') &&
					qttag != MAKEID('t', 'r', 'a', 'k') &&
					qttag != MAKEID('m', 'd', 'i', 'a') &&
					qttag != MAKEID('m', 'd', 'h', 'd') &&
					qttag != MAKEID('m', 'i', 'n', 'f') &&
					qttag != MAKEID('g', 'm', 'i', 'n') &&
 					qttag != MAKEID('d', 'i', 'n', 'f') &&
 					qttag != MAKEID('a', 'l', 'i', 's') &&
 					qttag != MAKEID('s', 't', 's', 'd') &&
 					qttag != MAKEID('s', 't', 'b', 'l') &&
 					qttag != MAKEID('s', 't', 't', 's') &&
 					qttag != MAKEID('s', 't', 's', 'c') &&
					qttag != MAKEID('s', 't', 's', 'z') &&
					qttag != MAKEID('s', 't', 'c', 'o') &&
 					qttag != MAKEID('c', 'o', '6', '4') &&
 					qttag != MAKEID('h', 'd', 'l', 'r'))
 				{
					LongSeek(mp4, qtsize - 8);
 
 					NESTSIZE(qtsize);
 				}
				else
#endif
					if (qttag == MAKEID('m', 'v', 'h', 'd')) //mvhd  movie header
					{
						len = fread(&skip, 1, 4, mp4->mediafp);
						len += fread(&skip, 1, 4, mp4->mediafp);
 						len += fread(&skip, 1, 4, mp4->mediafp);
 						len += fread(&mp4->clockdemon, 1, 4, mp4->mediafp); mp4->clockdemon = BYTESWAP32(mp4->clockdemon);
 						len += fread(&mp4->clockcount, 1, 4, mp4->mediafp); mp4->clockcount = BYTESWAP32(mp4->clockcount);

						mp4->filepos += len;
						LongSeek(mp4, qtsize - 8 - len); // skip over mvhd
 
 						NESTSIZE(qtsize);
 					}
					else if (qttag == MAKEID('m', 'd', 'h', 'd')) //mdhd  media header
					{
						media_header md;
						len = fread(&md, 1, sizeof(md), mp4->mediafp);
						if (len == sizeof(md))
						{
							md.creation_time = BYTESWAP32(md.creation_time);
							md.modification_time = BYTESWAP32(md.modification_time);
							md.time_scale = BYTESWAP32(md.time_scale);
							md.duration = BYTESWAP32(md.duration);

							mp4->trak_clockdemon = md.time_scale;
							mp4->trak_clockcount = md.duration;

							if (mp4->videolength == 0.0) // Get the video length from the first track
							{
 								mp4->videolength = (float)((double)mp4->trak_clockcount / (double)mp4->trak_clockdemon);
 							}
 						}

						mp4->filepos += len;
						LongSeek(mp4, qtsize - 8 - len); // skip over mvhd
 
 						NESTSIZE(qtsize);
 					}
					else if (qttag == MAKEID('h', 'd', 'l', 'r')) //hldr
					{
						uint32_t temp;
						len = fread(&skip, 1, 4, mp4->mediafp);
 						len += fread(&skip, 1, 4, mp4->mediafp);
 						len += fread(&temp, 1, 4, mp4->mediafp);  // type will be 'meta' for the correct trak.
 
						if (temp != MAKEID('a', 'l', 'i', 's') && temp != MAKEID('u', 'r', 'l', ' '))
 							type = temp;
 
						mp4->filepos += len;
						LongSeek(mp4, qtsize - 8 - len); // skip over hldr
 
 						NESTSIZE(qtsize);
 
					}
					else if (qttag == MAKEID('s', 't', 's', 'd')) //read the sample decription to determine the type of metadata
					{
						if (type == traktype) //like meta
						{
							len = fread(&skip, 1, 4, mp4->mediafp);
							len += fread(&skip, 1, 4, mp4->mediafp);
							len += fread(&skip, 1, 4, mp4->mediafp);
							len += fread(&subtype, 1, 4, mp4->mediafp);  // type will be 'meta' for the correct trak.
							if (len == 16)
							{
								if (subtype != traksubtype) // MP4 metadata 
								{
 									type = 0; // MP4
 								}
 							}
							mp4->filepos += len;
							LongSeek(mp4, qtsize - 8 - len); // skip over stsd
 						}
 						else
							LongSeek(mp4, qtsize - 8);
 
 						NESTSIZE(qtsize);
 					}
					else if (qttag == MAKEID('s', 't', 's', 'c')) // metadata stsc - offset chunks
					{
						if (type == traktype) // meta
						{
							len = fread(&skip, 1, 4, mp4->mediafp);
							len += fread(&num, 1, 4, mp4->mediafp);

							num = BYTESWAP32(num);
							if (num * 12 <= qtsize - 8 - len)
 							{
 								mp4->metastsc_count = num;
 								if (mp4->metastsc) free(mp4->metastsc);
								if (num > 0)
 								{
									mp4->metastsc = (SampleToChunk *)malloc(num * sizeof(SampleToChunk));
									if (mp4->metastsc)
 									{
										len += fread(mp4->metastsc, 1, num * sizeof(SampleToChunk), mp4->mediafp);
 
										do
										{
											num--;
											mp4->metastsc[num].chunk_num = BYTESWAP32(mp4->metastsc[num].chunk_num);
											mp4->metastsc[num].samples = BYTESWAP32(mp4->metastsc[num].samples);
											mp4->metastsc[num].id = BYTESWAP32(mp4->metastsc[num].id);
										} while (num > 0);
									}
								}
								else
 								{
									//size of null
									CloseSource((size_t)mp4);
									mp4 = NULL;
									break;
 								}
 							}
							mp4->filepos += len;
							LongSeek(mp4, qtsize - 8 - len); // skip over stsx
 						}
 						else
							LongSeek(mp4, qtsize - 8);
 
 						NESTSIZE(qtsize);
 					}
					else if (qttag == MAKEID('s', 't', 's', 'z')) // metadata stsz - sizes
					{
						if (type == traktype) // meta
						{
							uint32_t equalsamplesize;

							len = fread(&skip, 1, 4, mp4->mediafp);
							len += fread(&equalsamplesize, 1, 4, mp4->mediafp);
							len += fread(&num, 1, 4, mp4->mediafp);

							num = BYTESWAP32(num);
							if (num * 4 <= qtsize - 8 - len)
 							{
 								mp4->metasize_count = num;
 								if (mp4->metasizes) free(mp4->metasizes);
								if(num > 0)
 								{
									mp4->metasizes = (uint32_t *)malloc(num * 4);
									if (mp4->metasizes)
 									{
										if (equalsamplesize == 0)
 										{
											len += fread(mp4->metasizes, 1, num * 4, mp4->mediafp);
											do
											{
												num--;
												mp4->metasizes[num] = BYTESWAP32(mp4->metasizes[num]);
											} while (num > 0);
										}
										else
 										{
											equalsamplesize = BYTESWAP32(equalsamplesize);
											do
											{
												num--;
												mp4->metasizes[num] = equalsamplesize;
											} while (num > 0);
										}
 									}
 								}
								else
								{
									//size of null
									CloseSource((size_t)mp4);
									mp4 = NULL;
									break;
								}
 							}
							mp4->filepos += len;
							LongSeek(mp4, qtsize - 8 - len); // skip over stsz
 						}
 						else
							LongSeek(mp4, qtsize - 8);
 
 						NESTSIZE(qtsize);
 					}
					else if (qttag == MAKEID('s', 't', 'c', 'o')) // metadata stco - offsets
					{
						if (type == traktype) // meta
						{
							len = fread(&skip, 1, 4, mp4->mediafp);
							len += fread(&num, 1, 4, mp4->mediafp);
 							num = BYTESWAP32(num);
 							if (num * 4 <= qtsize - 8 - len)
 							{
								uint32_t metastco_count = num;

 								if (mp4->metastsc_count > 0 && num != mp4->metasize_count)
 								{
									mp4->indexcount = num;
 									if (mp4->metaoffsets) free(mp4->metaoffsets);
									if(num > 0)
 									{
										mp4->metaoffsets = (uint64_t *)malloc(num * 8);
										if (mp4->metaoffsets)
 										{
											uint32_t *metaoffsets32 = NULL;
											metaoffsets32 = (uint32_t *)malloc(num * 4);
											if (metaoffsets32)
 											{
												uint64_t fileoffset = 0;
												int stsc_pos = 0;
												int stco_pos = 0;
												int repeat = 1;
												len += fread(metaoffsets32, 1, num * 4, mp4->mediafp);
												do
 												{
													num--;
													metaoffsets32[num] = BYTESWAP32(metaoffsets32[num]);
												} while (num > 0);

												mp4->metaoffsets[0] = fileoffset = metaoffsets32[stco_pos];
												num = 1;
												while (num < mp4->indexcount)
 												{
													if ((uint32_t)repeat == mp4->metastsc[stsc_pos].samples)
													{
														if ((uint32_t)stco_pos + 1 < metastco_count)
														{
															stco_pos++;
															fileoffset = (uint64_t)metaoffsets32[stco_pos];
														}
														else
														{
															fileoffset += (uint64_t)mp4->metasizes[num - 1];
														}
														if ((uint32_t)stsc_pos + 1 < mp4->metastsc_count)
															if (mp4->metastsc[stsc_pos + 1].chunk_num == (uint32_t)stco_pos + 1)
																stsc_pos++;

														repeat = 1;
													}
													else
													{
														fileoffset += (uint64_t)mp4->metasizes[num - 1];
														repeat++;
													}

													mp4->metaoffsets[num] = fileoffset;
													//int delta = metaoffsets[num] - metaoffsets[num - 1];
													//printf("%3d:%08x, delta = %08x\n", num, (int)fileoffset, delta);

													num++;
 												}
 
												if (mp4->metastsc) free(mp4->metastsc);
												mp4->metastsc = NULL;
												mp4->metastsc_count = 0;
 
												free(metaoffsets32);
 											}
 										}
 									}
									else
									{
										//size of null
										CloseSource((size_t)mp4);
										mp4 = NULL;
										break;
									}
 								}
 								else
 								{
 									mp4->indexcount = num;
 									if (mp4->metaoffsets) free(mp4->metaoffsets);
									if (num > 0)
 									{
										mp4->metaoffsets = (uint64_t *)malloc(num * 8);
										if (mp4->metaoffsets)
 										{
											uint32_t *metaoffsets32 = NULL;
											metaoffsets32 = (uint32_t *)malloc(num * 4);
											if (metaoffsets32)
 											{
												size_t readlen = fread(metaoffsets32, 1, num * 4, mp4->mediafp);
												len += readlen;
												do
												{
													num--;
													mp4->metaoffsets[num] = BYTESWAP32(metaoffsets32[num]);
												} while (num > 0);
 
												free(metaoffsets32);
											}
 										}
 									}
									else
									{
										//size of null
										CloseSource((size_t)mp4);
										mp4 = NULL;
										break;
									}
 								}
 							}
							mp4->filepos += len;
							LongSeek(mp4, qtsize - 8 - len); // skip over stco
 						}
 						else
							LongSeek(mp4, qtsize - 8);
 
 						NESTSIZE(qtsize);
 					}

					else if (qttag == MAKEID('c', 'o', '6', '4')) // metadata stco - offsets
					{
						if (type == traktype) // meta
						{
 							len = fread(&skip, 1, 4, mp4->mediafp);
 							len += fread(&num, 1, 4, mp4->mediafp);
 							num = BYTESWAP32(num);

							if(num == 0)
							{
								//size of null
								CloseSource((size_t)mp4);
								mp4 = NULL;
								break;
							}

 							if (num * 8 <= qtsize - 8 - len)
 							{
 								if (mp4->metastsc_count > 0 && num != mp4->metasize_count)
 								{
 									mp4->indexcount = mp4->metasize_count;
 									if (mp4->metaoffsets) free(mp4->metaoffsets);
									if (mp4->metasize_count)
 									{
										mp4->metaoffsets = (uint64_t *)malloc(mp4->metasize_count * 8);
										if (mp4->metaoffsets)
 										{
											uint64_t *metaoffsets64 = NULL;
											metaoffsets64 = (uint64_t *)malloc(num * 8);
											if (metaoffsets64)
 											{
												uint64_t fileoffset = 0;
												int stsc_pos = 0;
												int stco_pos = 0;
												len += fread(metaoffsets64, 1, num * 8, mp4->mediafp);
												do
												{
													num--;
													metaoffsets64[num] = BYTESWAP64(metaoffsets64[num]);
												} while (num > 0);
 
												fileoffset = metaoffsets64[0];
												mp4->metaoffsets[0] = fileoffset;
												//printf("%3d:%08x, delta = %08x\n", 0, (int)fileoffset, 0);
 
												num = 1;
												while (num < mp4->metasize_count)
 												{
													if (num != mp4->metastsc[stsc_pos].chunk_num - 1 && 0 == (num - (mp4->metastsc[stsc_pos].chunk_num - 1)) % mp4->metastsc[stsc_pos].samples)
													{
														stco_pos++;
														fileoffset = (uint64_t)metaoffsets64[stco_pos];
													}
													else
													{
														fileoffset += (uint64_t)mp4->metasizes[num - 1];
													}

													mp4->metaoffsets[num] = fileoffset;
													//int delta = metaoffsets[num] - metaoffsets[num - 1];
													//printf("%3d:%08x, delta = %08x\n", num, (int)fileoffset, delta);

													num++;
 												}
 
												if (mp4->metastsc) free(mp4->metastsc);
												mp4->metastsc = NULL;
												mp4->metastsc_count = 0;
 
												free(metaoffsets64);
 											}
 										}
 									}
									else
									{
										//size of null
										CloseSource((size_t)mp4);
										mp4 = NULL;
										break;
									}
 								}
 								else
 								{
									mp4->indexcount = num;
									if (mp4->metaoffsets) free(mp4->metaoffsets);
									mp4->metaoffsets = (uint64_t *)malloc(num * 8);
									if (mp4->metaoffsets)
									{
										len += fread(mp4->metaoffsets, 1, num * 8, mp4->mediafp);
										do
										{
											num--;
											mp4->metaoffsets[num] = BYTESWAP64(mp4->metaoffsets[num]);
										} while (num > 0);
 									}
 								}
 							}
							mp4->filepos += len;
							LongSeek(mp4, qtsize - 8 - len); // skip over stco
 						}
 						else
							LongSeek(mp4, qtsize - 8);
 
 						NESTSIZE(qtsize);
 					}
					else if (qttag == MAKEID('s', 't', 't', 's')) // time to samples
					{
						if (type == traktype) // meta 
						{
							uint32_t totaldur = 0, samples = 0;
							int32_t entries = 0;
							len = fread(&skip, 1, 4, mp4->mediafp);
							len += fread(&num, 1, 4, mp4->mediafp);
							num = BYTESWAP32(num);
							if (num * 8 <= qtsize - 8 - len)
							{
								entries = num;

								mp4->meta_clockdemon = mp4->trak_clockdemon;
								mp4->meta_clockcount = mp4->trak_clockcount;

								while (entries > 0)
								{
									int32_t samplecount;
									int32_t duration;
									len += fread(&samplecount, 1, 4, mp4->mediafp);
									samplecount = BYTESWAP32(samplecount);
									len += fread(&duration, 1, 4, mp4->mediafp);
									duration = BYTESWAP32(duration);

									samples += samplecount;
									entries--;

									totaldur += duration;
									mp4->metadatalength += (double)((double)samplecount * (double)duration / (double)mp4->meta_clockdemon);
 								}
 								mp4->basemetadataduration = mp4->metadatalength * (double)mp4->meta_clockdemon / (double)samples;
 							}
							mp4->filepos += len;
							LongSeek(mp4, qtsize - 8 - len); // skip over stco
 						}
 						else
							LongSeek(mp4, qtsize - 8);
 
 						NESTSIZE(qtsize);
 					}
					else
					{
						NESTSIZE(8);
					}
			}
			else
			{
 				break;
 			}
 		} while (len > 0);

		if (mp4)
		{
			if (mp4->metasizes == NULL || mp4->metaoffsets == NULL)
			{
				CloseSource((size_t)mp4);
				mp4 = NULL;
			}
		}
 	}
 	else
 	{

		free(mp4);
		mp4 = NULL;
	}

	return (size_t)mp4;
}