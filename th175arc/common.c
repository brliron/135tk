#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// mkdir
#include <sys/stat.h>
#include <sys/types.h>
// PATH_MAX
#ifndef WIN32
#include <linux/limits.h>
#endif
//
#include "th175arc.h"

uint32_t calc_hash(const char *filename)
{
	if (strcmp(filename, "game.exe") == 0) {
		// I don't know the filename for this hash, but I need to have one, so I arbitrarily decide it's "game.exe".
		return 0x1f47c0c8;
	}

	uint64_t hash = 0x811C9DC5;
	for (int i = 0; filename[i]; i++) {
		hash = ((hash ^ filename[i]) * 0x1000193) & 0xFFFFFFFF;
	}
	return (uint32_t)hash;
}



// Asm-like version
void decrypt_file_asm(uint8_t *buffer, size_t size)
{
	uint8_t key[] = "z.e-ahwqb1neai un0dsk;afjv0cx0@z";
	uint8_t *buffer_ptr = buffer;
	uint8_t cl = 0;

	unsigned char key_offset; // edx
	for (size_t offset = 0; offset < size;) {
		key_offset = offset;
		cl += (offset & 0xFF);
		key_offset &= 0x1f;
		buffer_ptr++;
		offset++;
		cl ^= key[key_offset];
		buffer_ptr[-1] ^= cl;
		cl = 0; // The unknown stuff on stack
		buffer_ptr[-1] ^= (size & 0xFF);
	}
}

// Readable version
void decrypt_file(uint8_t *buffer, size_t size)
{
	uint8_t key[] = "z.e-ahwqb1neai un0dsk;afjv0cx0@z";
	uint8_t cl = 0;

	for (size_t offset = 0; offset < size; offset++) {
		cl = (offset & 0xFF) ^ key[offset & 0x1F];
		buffer[offset] ^= cl;
		buffer[offset] ^= (size & 0xFF);
	}
}

uint8_t *read_file(const char *path, size_t *size)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		perror(path);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t *buffer = malloc(*size);
	if (fread(buffer, *size, 1, file) == 0) {
		perror(path);
		fclose(file);
		free(buffer);
		return NULL;
	}

	fclose(file);
	return buffer;
}

int create_directory_for_path(const char *path_)
{
	char *path = strdup(path_);
	char *end = path;

	while (1) {
		end = strchr(end, '/');
		if (!end) {
			break;
		}
		*end = '\0';
#ifdef WIN32
		mkdir(path);
#else
		mkdir(path, 0755);
#endif
		*end = '/';
		end++;
	}
	free(path);
	return 0;
}

const char *make_path(const char *left, const char *right)
{
	static char static_path[PATH_MAX];
	snprintf(static_path, PATH_MAX, "%s/%s", left, right);
	return static_path;
}

int write_file(const char *path, const uint8_t *data, size_t size)
{
	create_directory_for_path(path);

	FILE *file = fopen(path, "wb");
	if (file == NULL) {
		perror(path);
		return 1;
	}

	if (size > 0 && fwrite(data, size, 1, file) == 0) {
		perror(path);
		fclose(file);
		return 1;
	}

	fclose(file);
	return 0;
}
