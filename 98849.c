load_map(const char* filename)
{

	uint16_t                 count;
	struct rmp_entity_header entity_hdr;
	file_t*                  file = NULL;
	bool                     has_failed;
	struct map_layer*        layer;
	struct rmp_layer_header  layer_hdr;
	struct map*              map = NULL;
	int                      num_tiles;
	struct map_person*       person;
	struct rmp_header        rmp;
	lstring_t*               script;
	rect_t                   segment;
	int16_t*                 tile_data = NULL;
	path_t*                  tileset_path;
	tileset_t*               tileset;
	struct map_trigger       trigger;
	struct map_zone          zone;
	struct rmp_zone_header   zone_hdr;
	lstring_t*               *strings = NULL;

	int i, j, x, y, z;

	console_log(2, "constructing new map from '%s'", filename);

	memset(&rmp, 0, sizeof(struct rmp_header));

	if (!(file = file_open(g_game, filename, "rb")))
		goto on_error;
	map = calloc(1, sizeof(struct map));
	if (file_read(file, &rmp, 1, sizeof(struct rmp_header)) != 1)
		goto on_error;
	if (memcmp(rmp.signature, ".rmp", 4) != 0) goto on_error;
	if (rmp.num_strings != 3 && rmp.num_strings != 5 && rmp.num_strings < 9)
		goto on_error;
	if (rmp.start_layer < 0 || rmp.start_layer >= rmp.num_layers)
		rmp.start_layer = 0;  // being nice here, this really should fail outright
	switch (rmp.version) {
	case 1:
		strings = calloc(rmp.num_strings, sizeof(lstring_t*));
		has_failed = false;
		for (i = 0; i < rmp.num_strings; ++i)
			has_failed = has_failed || ((strings[i] = read_lstring(file, true)) == NULL);
		if (has_failed) goto on_error;

		map->layers = calloc(rmp.num_layers, sizeof(struct map_layer));
		map->persons = calloc(rmp.num_entities, sizeof(struct map_person));
		map->triggers = vector_new(sizeof(struct map_trigger));
		map->zones = vector_new(sizeof(struct map_zone));

		for (i = 0; i < rmp.num_layers; ++i) {
			if (file_read(file, &layer_hdr, 1, sizeof(struct rmp_layer_header)) != 1)
				goto on_error;
			layer = &map->layers[i];
			layer->is_parallax = (layer_hdr.flags & 2) != 0x0;
			layer->is_reflective = layer_hdr.is_reflective;
			layer->is_visible = (layer_hdr.flags & 1) == 0x0;
			layer->color_mask = mk_color(255, 255, 255, 255);
			layer->width = layer_hdr.width;
			layer->height = layer_hdr.height;
			layer->autoscroll_x = layer->is_parallax ? layer_hdr.scrolling_x : 0.0;
			layer->autoscroll_y = layer->is_parallax ? layer_hdr.scrolling_y : 0.0;
			layer->parallax_x = layer->is_parallax ? layer_hdr.parallax_x : 1.0;
			layer->parallax_y = layer->is_parallax ? layer_hdr.parallax_y : 1.0;
			if (!layer->is_parallax) {
				map->width = fmax(map->width, layer->width);
				map->height = fmax(map->height, layer->height);
			}
			if (!(layer->tilemap = malloc(layer_hdr.width * layer_hdr.height * sizeof(struct map_tile))))
				goto on_error;
			layer->name = read_lstring(file, true);
			layer->obsmap = obsmap_new();
			num_tiles = layer_hdr.width * layer_hdr.height;
			if ((tile_data = malloc(num_tiles * 2)) == NULL)
				goto on_error;
			if (file_read(file, tile_data, num_tiles, 2) != num_tiles)
				goto on_error;
			for (j = 0; j < num_tiles; ++j)
				layer->tilemap[j].tile_index = tile_data[j];
			for (j = 0; j < layer_hdr.num_segments; ++j) {
				if (!fread_rect32(file, &segment)) goto on_error;
				obsmap_add_line(layer->obsmap, segment);
			}
			free(tile_data);
			tile_data = NULL;
		}

		if (map->width == 0 || map->height == 0)
			goto on_error;

		map->num_persons = 0;
		for (i = 0; i < rmp.num_entities; ++i) {
			if (file_read(file, &entity_hdr, 1, sizeof(struct rmp_entity_header)) != 1)
				goto on_error;
			if (entity_hdr.z < 0 || entity_hdr.z >= rmp.num_layers)
				entity_hdr.z = 0;
			switch (entity_hdr.type) {
			case 1:  // person
				++map->num_persons;
				person = &map->persons[map->num_persons - 1];
				memset(person, 0, sizeof(struct map_person));
				if (!(person->name = read_lstring(file, true)))
					goto on_error;
				if (!(person->spriteset = read_lstring(file, true)))
					goto on_error;
				person->x = entity_hdr.x; person->y = entity_hdr.y; person->z = entity_hdr.z;
				if (file_read(file, &count, 1, 2) != 1 || count < 5)
					goto on_error;
				person->create_script = read_lstring(file, false);
				person->destroy_script = read_lstring(file, false);
				person->touch_script = read_lstring(file, false);
				person->talk_script = read_lstring(file, false);
				person->command_script = read_lstring(file, false);
				for (j = 5; j < count; ++j)
					lstr_free(read_lstring(file, true));
				file_seek(file, 16, WHENCE_CUR);
				break;
			case 2:  // trigger
				if ((script = read_lstring(file, false)) == NULL) goto on_error;
				memset(&trigger, 0, sizeof(struct map_trigger));
				trigger.x = entity_hdr.x;
				trigger.y = entity_hdr.y;
				trigger.z = entity_hdr.z;
				trigger.script = script_new(script, "%s/trig%d", filename, vector_len(map->triggers));
				if (!vector_push(map->triggers, &trigger))
					return false;
				lstr_free(script);
				break;
			default:
				goto on_error;
			}
		}

		for (i = 0; i < rmp.num_zones; ++i) {
			if (file_read(file, &zone_hdr, 1, sizeof(struct rmp_zone_header)) != 1)
				goto on_error;
			if ((script = read_lstring(file, false)) == NULL) goto on_error;
			if (zone_hdr.layer < 0 || zone_hdr.layer >= rmp.num_layers)
				zone_hdr.layer = 0;
			zone.layer = zone_hdr.layer;
			zone.bounds = mk_rect(zone_hdr.x1, zone_hdr.y1, zone_hdr.x2, zone_hdr.y2);
			zone.interval = zone_hdr.interval;
			zone.steps_left = 0;
			zone.script = script_new(script, "%s/zone%d", filename, vector_len(map->zones));
			rect_normalize(&zone.bounds);
			if (!vector_push(map->zones, &zone))
				return false;
			lstr_free(script);
		}

		if (strcmp(lstr_cstr(strings[0]), "") != 0) {
			tileset_path = path_strip(path_new(filename));
			path_append(tileset_path, lstr_cstr(strings[0]));
			tileset = tileset_new(path_cstr(tileset_path));
			path_free(tileset_path);
		}
		else {
			tileset = tileset_read(file);
		}
		if (tileset == NULL) goto on_error;

		for (z = 0; z < rmp.num_layers; ++z) {
			layer = &map->layers[z];
			for (x = 0; x < layer->width; ++x) for (y = 0; y < layer->height; ++y) {
				i = x + y * layer->width;
				map->layers[z].tilemap[i].frames_left =
					tileset_get_delay(tileset, map->layers[z].tilemap[i].tile_index);
			}
		}

		map->bgm_file = strcmp(lstr_cstr(strings[1]), "") != 0
			? lstr_dup(strings[1]) : NULL;
		map->num_layers = rmp.num_layers;
		map->is_repeating = rmp.repeat_map;
		map->origin.x = rmp.start_x;
		map->origin.y = rmp.start_y;
		map->origin.z = rmp.start_layer;
		map->tileset = tileset;
		if (rmp.num_strings >= 5) {
			map->scripts[MAP_SCRIPT_ON_ENTER] = script_new(strings[3], "%s/onEnter", filename);
			map->scripts[MAP_SCRIPT_ON_LEAVE] = script_new(strings[4], "%s/onLeave", filename);
		}
		if (rmp.num_strings >= 9) {
			map->scripts[MAP_SCRIPT_ON_LEAVE_NORTH] = script_new(strings[5], "%s/onLeave", filename);
			map->scripts[MAP_SCRIPT_ON_LEAVE_EAST] = script_new(strings[6], "%s/onLeaveEast", filename);
			map->scripts[MAP_SCRIPT_ON_LEAVE_SOUTH] = script_new(strings[7], "%s/onLeaveSouth", filename);
			map->scripts[MAP_SCRIPT_ON_LEAVE_WEST] = script_new(strings[8], "%s/onLeaveWest", filename);
		}
		for (i = 0; i < rmp.num_strings; ++i)
			lstr_free(strings[i]);
		free(strings);
		break;
	default:
		goto on_error;
	}
	file_close(file);
	return map;

on_error:
	if (file != NULL) file_close(file);
	free(tile_data);
	if (strings != NULL) {
		for (i = 0; i < rmp.num_strings; ++i) lstr_free(strings[i]);
		free(strings);
	}
	if (map != NULL) {
		if (map->layers != NULL) {
			for (i = 0; i < rmp.num_layers; ++i) {
				lstr_free(map->layers[i].name);
				free(map->layers[i].tilemap);
				obsmap_free(map->layers[i].obsmap);
			}
			free(map->layers);
		}
		if (map->persons != NULL) {
			for (i = 0; i < map->num_persons; ++i) {
				lstr_free(map->persons[i].name);
				lstr_free(map->persons[i].spriteset);
				lstr_free(map->persons[i].create_script);
				lstr_free(map->persons[i].destroy_script);
				lstr_free(map->persons[i].command_script);
				lstr_free(map->persons[i].talk_script);
				lstr_free(map->persons[i].touch_script);
			}
			free(map->persons);
		}
		vector_free(map->triggers);
		vector_free(map->zones);
		free(map);
	}
	return NULL;
}
