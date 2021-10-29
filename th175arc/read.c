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



#define IMUL_1(x) imul(&edx, &eax, eax, x);
#define IMUL_3(x, y, z) imul(NULL, &x, y, z)
void imul(uint32_t *dst_high, uint32_t *dst_low, uint32_t src_1, uint32_t src_2)
{
	int64_t ret = ((int64_t)(int32_t)src_1) * (int32_t)src_2;
	if (dst_high) {
		*dst_high = ret >> 32;
	}
	*dst_low = (uint32_t)ret;
}

void decrypt(uint8_t *buffer, size_t size, size_t offset_in_file)
{
	uint32_t *buffer_int_array = (uint32_t*)buffer;
	uint32_t eax, ebx, ecx, edx, /* ebp, */ esi, edi;

	// uint32_t param1 = (uint32_t)buffer;
	uint32_t param2 = 0;
	uint32_t param3 = size;
	// Param 4 is an object with the header size and the file size

	// eax = param4
	edx = param3;
	if (1) {
		ecx = size;
		ecx ^= offset_in_file; // from param4
		edx = param3 >> 2;
	}

	eax = param2 >> 2;
	// ebp = param1;
	ecx += eax;

	while (edx > 0) {
		uint32_t stack_1 = edx;
		eax = ecx;
		edx = 0x5E4789C9;
		esi = ecx;
		edi = 0x5E4789C9;

		// copy
		IMUL_1(edx);
		eax = edx;
		edx = (int32_t)edx >> 0x0E;
		eax >>= 0x1F;
		edx += eax;
		IMUL_3(eax, edx, 0xADC8);
		IMUL_3(edx, edx, 0xFFFFF2B9);
		esi -= eax;
		IMUL_3(eax, esi, 0xBC8F);
		ebx = eax + edx;
		esi = eax + edx + 0x7FFFFFFF;

		if ((int32_t)ebx > 0) {
			esi = ebx;
		}
		eax = esi;
		ebx = esi;

		// paste
		IMUL_1(edi); // edi instead of edx
		eax = edx;
		edx = (int32_t)edx >> 0x0E;
		eax >>= 0x1F;
		edx += eax;
		IMUL_3(eax, edx, 0xADC8);
		IMUL_3(edx, edx, 0xFFFFF2B9);
		ebx -= eax; // ebx instead of esi
		IMUL_3(eax, ebx, 0xBC8F); // same
		edi = eax + edx; // edi instead of ebx
		ebx = eax + edx + 0x7FFFFFFF; // ebx instead of edi

		edx = 0x5E4789C9;
		if ((int32_t)edi > 0) {
			ebx = edi;
		}

		esi <<= 8;
		eax = ebx;
		edi = ebx & 0xFF;

		// paste
		IMUL_1(edx); // edx instead of edi
		edi |= esi; // Added in the middle
		eax = edx;
		edx = (int32_t)edx >> 0x0E;
		eax >>= 0x1F;
		edx += eax;
		IMUL_3(eax, edx, 0xADC8);
		IMUL_3(edx, edx, 0xFFFFF2B9);
		ebx -= eax; // ebx instead of esi
		IMUL_3(eax, ebx, 0xBC8F); // same
		esi = eax + edx; // esi instead of ebx
		ebx = eax + edx + 0x7FFFFFFF; // ebx instead of edi

		edx = 0x5E4789C9;
		if ((int32_t)esi > 0) {
			ebx = esi;
		}

		edi <<= 8;
		eax = ebx;
		esi = ebx & 0xFF;

		// paste
		IMUL_1(edx); // edx instead of edi
		esi |= edi; // Added in the middle
		eax = edx;
		edx = (int32_t)edx >> 0x0E;
		eax >>= 0x1F;
		edx += eax;
		IMUL_3(eax, edx, 0xADC8);
		IMUL_3(edx, edx, 0xFFFFF2B9);
		ebx -= eax; // ebx instead of esi
		IMUL_3(eax, ebx, 0xBC8F); // same
		edi = eax + edx; // edi instead of ebx
		eax = eax + edx + 0xFF; // Completely different

		edx = stack_1;
		if ((int32_t)edi > 0) {
			eax = edi;
		}
		esi <<= 8;
		ecx++;
		eax &= 0xFF;
		eax |= esi;
		*buffer_int_array ^= eax;
		buffer_int_array++;
		edx--;
	}
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
