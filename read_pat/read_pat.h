#ifndef READ_PAT_H_
# define READ_PAT_H_

# include <stdio.h>
# include <stdint.h>
# include <jansson.h>

void	part1(FILE* fp, char* pat_path, json_t *js);
void	textures(FILE* fp, char* pat_path, json_t *js);
void    take_array(FILE* fp, char* pat_path, json_t *js);
void	part4(FILE* fp, char* pat_path, json_t *js);

uint8_t  read_u8(FILE* fp, json_t *js, const char *name);
uint16_t read_u16(FILE* fp, json_t *js, const char *name);
uint32_t read_u32(FILE* fp, json_t *js, const char *name);
uint64_t read_u64(FILE* fp, json_t *js, const char *name);
int8_t  read_i8(FILE* fp, json_t *js, const char *name);
int16_t read_i16(FILE* fp, json_t *js, const char *name);
int32_t read_i32(FILE* fp, json_t *js, const char *name);
int64_t read_i64(FILE* fp, json_t *js, const char *name);
float read_float(FILE* fp, json_t *js, const char *name);
void read_str(FILE* fp, char* buffer, size_t size, json_t *js, const char *name);
void read_bytes(FILE* fp, void* buffer, size_t size, json_t *js, const char *name);

const char* img_idx_to_name(int idx);

json_t* js_enter(json_t *parent, const char* name);
const char* idx_to_str(const char* base, int i);

#endif /* !READ_PAT_H_ */
