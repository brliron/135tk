#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
// FindFirstFile
# include <windows.h>
#else
// opendir
# include <sys/types.h>
# include <dirent.h>
#endif
//
#include "th175arc.h"

char **add_to_list(char **list_in, size_t *size, size_t *capacity, char *elem)
{
	if (*size + 1 > *capacity) {
		if (*capacity > 0) {
			*capacity *= 2;
		}
		else {
			*capacity = 64;
		}
		list_in = realloc(list_in, *capacity * sizeof(char*));
	}
	list_in[*size] = elem;
	(*size)++;
	return list_in;
}

#ifdef WIN32
char **list_files(const char *in_dir)
{
	char pattern[MAX_PATH];
	snprintf(pattern, MAX_PATH, "%s\\*", in_dir);

	WIN32_FIND_DATA entry;
	HANDLE hFind = FindFirstFile(pattern, &entry);
	if (hFind == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "%s: Windows error code %lu\n", pattern, GetLastError());
		return NULL;
	}

	char **list = NULL;
	size_t size = 0;
	size_t capacity = 0;
	do {
		char *path = strdup(make_path(in_dir, entry.cFileName));
		if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			list = add_to_list(list, &size, &capacity, path);
			// Do not free path
		}
		else if ((entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			strcmp(entry.cFileName, ".") != 0 &&
			strcmp(entry.cFileName, "..") != 0) {
			char **sublist = list_files(path);
			if (sublist) {
				for (int i = 0; sublist[i]; i++) {
					list = add_to_list(list, &size, &capacity, sublist[i]);
				}
				free(sublist);
			}
			free(path);
		}
		else {
			// Unknown file type
			free(path);
		}
	} while (FindNextFile(hFind, &entry));

	FindClose(hFind);
	list = add_to_list(list, &size, &capacity, NULL);
	return list;
}
#else
char **list_files(const char *in_dir)
{
	DIR *dir = opendir(in_dir);
	if (dir == NULL) {
		perror(in_dir);
		return NULL;
	}

	char **list = NULL;
	size_t size = 0;
	size_t capacity = 0;
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		char *path = strdup(make_path(in_dir, entry->d_name));
		if (entry->d_type == DT_REG) {
			list = add_to_list(list, &size, &capacity, path);
			// Do not free path
		}
		else if (entry->d_type == DT_DIR &&
			strcmp(entry->d_name, ".") != 0 &&
			strcmp(entry->d_name, "..") != 0) {
			char **sublist = list_files(path);
			if (sublist) {
				for (int i = 0; sublist[i]; i++) {
					list = add_to_list(list, &size, &capacity, sublist[i]);
				}
				free(sublist);
			}
			free(path);
		}
		else {
			// Unknown file type
			free(path);
		}
	}

	closedir(dir);
	list = add_to_list(list, &size, &capacity, NULL);
	return list;
}
#endif /* WIN32 */

void free_files_list(char **list)
{
	for (int i = 0; list[i]; i++) {
		free(list[i]);
	}
	free(list);
}

// We always want to write game.exe first if it exists
// so we move it at the top of the list.
void move_game_exe_first(char **list, const char *in_dir)
{
	// If game.exe is already first, no need to move it
	for (int i = 1; list[i]; i++) {
		if (strcmp(list[i] + strlen(in_dir) + strlen("/"), "game.exe") == 0) {
			// TODO: test
			printf("Moving %s to 1st position\n", list[i]);
			char *tmp = list[0];
			list[0] = list[i];
			list[i] = tmp;
		}
	}
}

int repack_file(const char *in_dir,  const char *out_file)
{
	char **files_list = list_files(in_dir);
	if (files_list == NULL) {
		return 1;
	}
	move_game_exe_first(files_list, in_dir);

	size_t files_count;
	for (files_count = 0; files_list[files_count]; files_count++) {
	}
	file_desc_t *desc = malloc(files_count * sizeof(file_desc_t));

	FILE *out = fopen(out_file, "wb");
	if (out == NULL) {
		perror(out_file);
		free(desc);
		free_files_list(files_list);
		return 1;
	}

	size_t offset = 0;
	for (size_t i = 0; files_list[i]; i++) {
		size_t size;
		uint8_t *file = read_file(files_list[i], &size);
		if (file == NULL) {
			continue;
		}

		desc[i].key = calc_hash(files_list[i] + strlen(in_dir) + strlen("/"));
		desc[i].offset = offset;
		desc[i].size = size;
        if (desc[i].key != GAME_EXE_HASH) {
            decrypt(file, size, offset);
        }
		offset += size;
		fwrite(file, size, 1, out);
		free(file);
	}

	size_t desc_size = sizeof(file_desc_t) * files_count;
	decrypt((uint8_t*)desc, desc_size, offset);
	offset += desc_size;
	fwrite(desc, desc_size, 1, out);
	free(desc);

	file_footer_t footer;
	footer.file_desc_size = sizeof(file_desc_t);
	footer.nb_files = files_count;
	footer.footer_size = sizeof(file_footer_t);
	footer.unk1 = 0;
	footer.unk2 = 0;
	footer.unk3 = 0; // Have a non-zero value in the game's original files, but seems to work with zero
	footer.unk4 = 0;
	footer.unk5 = 0;
	decrypt((uint8_t*)&footer, sizeof(file_footer_t), offset);
	fwrite(&footer, sizeof(file_footer_t), 1, out);

	fclose(out);
	free_files_list(files_list);
	return 0;
}
