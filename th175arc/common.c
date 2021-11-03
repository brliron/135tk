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

// ASM-like version
void decrypt_asm(uint8_t *buffer, size_t size, size_t offset_in_file)
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

void do_partial_xor(uint8_t *dst, uint8_t *src, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        dst[i] ^= src[i];
    }
}

// More readable version (well, not really yet)
void decrypt(uint8_t *buffer, size_t size, size_t offset_in_file)
{
	uint32_t *buffer_int_array = (uint32_t*)buffer;
	uint32_t eax, ebx, ecx, edx, esi, edi;

	ecx = size ^ offset_in_file;
    while (size > 0) {
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

		if ((int32_t)edi > 0) {
			eax = edi;
		}
		esi <<= 8;

		ecx++;
		eax &= 0xFF;
		eax |= esi;
        if (size >= 4) {
            *buffer_int_array ^= eax;
        }
        else {
            do_partial_xor((uint8_t*)buffer_int_array, (uint8_t*)&eax, size);
        }
		buffer_int_array++;
		if (size < 4) {
            break;
        }
        size -= 4;
	}
}
#undef IMUL_1
#undef IMUL_3

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
