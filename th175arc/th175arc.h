#ifndef TH175ARC_H_
# define TH175ARC_H_

# include <stddef.h>
# include <stdint.h>

typedef struct file_footer_s
{
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3; // Non-zero (4 bytes filled)
	uint32_t unk4;
	uint32_t unk5;
	uint32_t file_desc_size; // 0x18
	uint32_t nb_files;
	uint32_t footer_size; // must be 0x20
} file_footer_t;

typedef struct file_desc_s
{
	uint64_t key;
	uint64_t offset;
	uint64_t size;
} file_desc_t;

int unpack_file(const char *in_file, const char *out_dir);
int repack_file(const char *in_dir,  const char *out_file);
#define GAME_EXE_HASH 0x1f47c0c8

uint32_t calc_hash(const char *filename);
void decrypt(uint8_t *buffer, size_t size, size_t offset_in_file);
const char *make_path(const char *left, const char *right);
uint8_t *read_file(const char *path, size_t *size);
int write_file(const char *path, const uint8_t *data, size_t size);

#endif /* !TH175ARC_H_ */
