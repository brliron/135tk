#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// filenames cache
#include <jansson.h>
//
#include "th175arc.h"

typedef struct filenames_cache_s
{
	uint32_t hash;
	char *filename;
} filenames_cache_t;

filenames_cache_t *filenames_cache_build(const char *path)
{
	json_t *list = json_load_file(path, 0, NULL);
	if (list == NULL) {
		fprintf(stderr, "%s: loading failed.\n", path);
		return NULL;
	}
	if (!json_is_array(list)) {
		fprintf(stderr, "%s is not an array.\n", path);
		json_decref(list);
		return NULL;
	}

	filenames_cache_t *cache = malloc((json_array_size(list) + 1) * sizeof(filenames_cache_t));
	size_t i;
	json_t *value;
	json_array_foreach(list, i, value) {
		char *fn = strdup(json_string_value(value));
		for (int j = 0; fn[j]; j++) {
			if (fn[j] == '\\') {
				fn[j] = '/';
			}
		}
		cache[i].hash = calc_hash(fn);
		cache[i].filename = fn;
	}
	cache[i].hash = 0;
	cache[i].filename = NULL;

	json_decref(list);
	return cache;
}

const char *filenames_cache_get(const filenames_cache_t *cache, uint32_t hash)
{
	if (!cache) return NULL;

	for (int i = 0; cache[i].hash; i++) {
		if (hash == cache[i].hash) {
			return cache[i].filename;
		}
	}
	return NULL;
}

void filenames_cache_free(filenames_cache_t *cache)
{
	if (!cache) return ;

	for (int i = 0; cache[i].hash; i++) {
		free(cache[i].filename);
	}
	free(cache);
}



int write_dat_file(filenames_cache_t *filenames_cache, const char *out_dir, uint32_t hash, const uint8_t *data, size_t size, size_t offset_in_file)
{
	uint8_t *buffer = malloc(size);
	memcpy(buffer, data, size);
	decrypt(buffer, size, offset_in_file);

	char name_buffer[26];
	const char *name = filenames_cache_get(filenames_cache, hash);
	if (name == NULL) {
		snprintf(name_buffer, 26, "unk/%x", hash);
		if (memcmp(buffer, "\x89PNG", 4) == 0) {
			strcat(name_buffer, ".png");
		}
		else if (memcmp(buffer, "GIF", 3) == 0) {
			strcat(name_buffer, ".gif");
		}
		else if (memcmp(buffer, "\xFA\xFARIQS", 6) == 0) {
			strcat(name_buffer, ".nut");
		}
		else if (memcmp(buffer, "RIFF", 4) == 0) {
			strcat(name_buffer, ".wav");
		}
		else if (memcmp(buffer, "OggS", 4) == 0) {
			strcat(name_buffer, ".ogg");
		}
		else if (memcmp(buffer, "OTTO", 4) == 0) {
			strcat(name_buffer, ".otf");
		}
		else if (memcmp(buffer, "\xEF\xBB\xBF", 3) == 0) {
			strcat(name_buffer, ".txt");
		}
		name = name_buffer;
	}

	int ret = write_file(make_path(out_dir, name), buffer, size);
	free(buffer);
	return ret;
}

int unpack_file(const char *in_file, const char *out_dir)
{
	if (out_dir == NULL) {
		out_dir = ".";
	}

	filenames_cache_t *filenames_cache = filenames_cache_build("fileslist.js");
	// Ignore filenames_cache_build errors

	size_t in_size;
	uint8_t *in_buffer = read_file(in_file, &in_size);
	if (in_buffer == NULL) {
		filenames_cache_free(filenames_cache);
		return 1;
	}

	decrypt(in_buffer + in_size - sizeof(file_footer_t), sizeof(file_footer_t), in_size - sizeof(file_footer_t));

	file_footer_t *file_footer = (file_footer_t*)(in_buffer + in_size - sizeof(file_footer_t));
	if (file_footer->footer_size != 0x20 ||
		sizeof(file_desc_t) != 0x18 ||
		file_footer->file_desc_size != 0x18) {
		fprintf(stderr, "%s: invalid cga archive\n", in_file);
		free(in_buffer);
		filenames_cache_free(filenames_cache);
		return 1;
	}

	size_t file_desc_table_size = sizeof(file_desc_t) * file_footer->nb_files;
	size_t file_desc_table_offset = in_size - sizeof(file_footer_t) - file_desc_table_size;
	decrypt(in_buffer + file_desc_table_offset, file_desc_table_size, file_desc_table_offset);
	file_desc_t *file_desc_table = (file_desc_t*)(in_buffer + file_desc_table_offset);
	for (size_t i = 0; i < file_footer->nb_files; i++) {
		file_desc_t *desc = &file_desc_table[i];
		write_dat_file(filenames_cache, out_dir, desc->key, &in_buffer[desc->offset], desc->size, desc->offset);
	}

	free(in_buffer);
	filenames_cache_free(filenames_cache);
	return 0;
}
