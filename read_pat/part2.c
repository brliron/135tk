#include <stdlib.h>
#include <string.h>
#include "read_pat.h"

#pragma pack(push, 1)
struct tfbm_header
{
  uint8_t	dontcare[18];
  uint32_t	size;
};
#pragma pack(pop)

static char* names[255] = { 0 };
const char* img_idx_to_name(int idx)
{
  if (idx >= 0 && names[idx])
    return names[idx];
  else
    return "null";
}

static void	type0(FILE* fp, int i, json_t *js)
{
  uint32_t	path_size = read_u32(fp, js, "path_size");
  char		path[256];

  read_str(fp, path, path_size, js, "path");
  path[path_size] = '\0';
  names[i] = strdup(path);

  printf("path=%s\n", path);
}

static void	type1(FILE* fp, int i, json_t *js)
{
  uint32_t	path_size = read_u32(fp, js, "path_size");
  char		path[256];
  uint16_t	coord[4];

  read_str(fp, path, path_size, js, "path");
  path[path_size] = '\0';
  names[i] = strdup(path);

  coord[0] = read_u16(fp, js, "x");
  coord[1] = read_u16(fp, js, "y");
  coord[2] = read_u16(fp, js, "w");
  coord[3] = read_u16(fp, js, "h");

  printf("path=%s, ", path);
  printf("x=%.3u y=%.3u w=%.3u h=%.3u\n", coord[0], coord[1], coord[2], coord[3]);
}


static void		type2(FILE* fp, char* pat_path, int i, json_t *js)
{
  struct tfbm_header	h;
  uint8_t*		data;
  char			out_path[256];
  FILE*			out;

  read_bytes(fp, &h, sizeof(h), js, "tfbm_header");
  data = malloc(h.size);
  read_bytes(fp, data, h.size, js, "tfbm_data");

  strcpy(out_path, pat_path);
  sprintf(out_path + strlen(out_path), "_%d.png", i);
  names[i] = strdup(out_path);

  out = fopen(out_path, "wb");
  fwrite(&h, sizeof(h), 1, out);
  fwrite(data, h.size, 1, out);
  free(data);

  printf("dumped in %s\n", out_path);
  fclose(out);
}

static void	type4(FILE* fp, int i, json_t *js)
{
  uint32_t	path_size = read_u32(fp, js, "path_size");
  char		path[256];
  uint32_t	entry_footer[4];

  read_str(fp, path, path_size, js, "path");
  path[path_size] = '\0';
  names[i] = strdup(path);

  entry_footer[0] = read_u32(fp, js, "unk1");
  entry_footer[1] = read_u32(fp, js, "unk2");
  entry_footer[2] = read_u32(fp, js, "unk3");
  entry_footer[3] = read_u32(fp, js, "unk4");

  printf("path=%s, ", path);
  printf("unk=%04u %04u %04u %04u\n", entry_footer[0], entry_footer[1], entry_footer[2], entry_footer[3]);
}

static void	type7(FILE* fp, json_t *js)
{
  uint8_t	b = read_u8(fp, js, "b");

  printf("b=%u\n", b);
}

static void	type128(FILE* fp, int i, json_t *js)
{
  uint32_t	path_size = read_u32(fp, js, "path_size");
  char		path[256];

  read_str(fp, path, path_size, js, "path");
  path[path_size] = '\0';
  names[i] = strdup(path);

  printf("path=%s\n", path);
}

static void	entry(FILE* fp, char* pat_path, unsigned int entry_index, json_t *js)
{
  uint8_t	unk1 = read_u8(fp, js, "unk1");
  printf("  entry %d: unk1 = 0x%.2x\n", entry_index + 1, unk1);
  uint16_t	nb_elems = read_u16(fp, js, "nb_elems");
  unsigned int	i = 0;
  while (i < nb_elems)
    {
      json_t*	elem = js_enter(js, idx_to_str("elem_", i + 1));
      uint8_t	type = read_u8(fp, elem, "type");
      printf("    %.4d: type=%u, ", i, type);
      if (type == 0)
	type0(fp, i, elem);
      else if (type == 1)
	type1(fp, i, elem);
      else if (type == 2)
	type2(fp, pat_path, i, elem);
      else if (type == 4)
	type4(fp, i, elem);
      else if (type == 7)
	type7(fp, elem);
      else if (type == 128)
	type128(fp, i, elem);
      else
	{
	  printf("Unknown type %d\n", type);
	  break ;
	}
      i++;
    }
}

void	textures(FILE* fp, char* pat_path, json_t *js)
{
  uint8_t	nb_texture = read_u8(fp, js, "nb_texture");
  printf("textures: %d entries\n", nb_texture);
  unsigned int	i = 0;
  while (i < nb_texture)
    {
      entry(fp, pat_path, i, js_enter(js, idx_to_str("", i + 1)));
      i++;
    }
}
