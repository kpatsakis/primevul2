static void rd_release_device_space(struct rd_dev *rd_dev)
static u32 rd_release_sgl_table(struct rd_dev *rd_dev, struct rd_dev_sg_table *sg_table,
				 u32 sg_table_count)
 {
 	struct page *pg;
 	struct scatterlist *sg;
	u32 i, j, page_count = 0, sg_per_table;
 
	for (i = 0; i < sg_table_count; i++) {
 		sg = sg_table[i].sg_table;
 		sg_per_table = sg_table[i].rd_sg_count;
 
		for (j = 0; j < sg_per_table; j++) {
			pg = sg_page(&sg[j]);
			if (pg) {
				__free_page(pg);
 				page_count++;
 			}
 		}
 		kfree(sg);
 	}
 
	kfree(sg_table);
	return page_count;
}

static void rd_release_device_space(struct rd_dev *rd_dev)
{
	u32 page_count;

	if (!rd_dev->sg_table_array || !rd_dev->sg_table_count)
		return;

	page_count = rd_release_sgl_table(rd_dev, rd_dev->sg_table_array,
					  rd_dev->sg_table_count);

 	pr_debug("CORE_RD[%u] - Released device space for Ramdisk"
 		" Device ID: %u, pages %u in %u tables total bytes %lu\n",
 		rd_dev->rd_host->rd_host_id, rd_dev->rd_dev_id, page_count,
 		rd_dev->sg_table_count, (unsigned long)page_count * PAGE_SIZE);
 
 	rd_dev->sg_table_array = NULL;
 	rd_dev->sg_table_count = 0;
 }