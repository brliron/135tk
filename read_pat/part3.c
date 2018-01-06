#include <stdlib.h>
#include <string.h>
#include "read_pat.h"

uint32_t	bswap(uint32_t in)
{
  uint32_t	out;
  uint8_t*	pIn  = (uint8_t*)&in;
  uint8_t*	pOut = (uint8_t*)&out;

  pOut[0] = pIn[3];
  pOut[1] = pIn[2];
  pOut[2] = pIn[1];
  pOut[3] = pIn[0];

  return out;
}

float	intToFloat(uint32_t in)
{
  float		out;
  uint8_t*	pIn  = (uint8_t*)&in;
  uint8_t*	pOut = (uint8_t*)&out;

  pOut[0] = pIn[0];
  pOut[1] = pIn[1];
  pOut[2] = pIn[2];
  pOut[3] = pIn[3];

  return out;
}

static void     frame(FILE* fp, unsigned int entry_index, json_t* js)
{
  uint16_t	unk1 = read_u16(fp, js, "unk1");
  uint8_t	unk2[0x30];
  read_bytes(fp, unk2, 0x30, js, "unk2");
  uint64_t	unk3 = read_u64(fp, js, "unk3");
  uint8_t	unk4 = read_u8(fp, js, "unk4");

  printf("    frame %d: unk1 = %d,\n      unk2 = 0x", entry_index + 1, unk1);
  unsigned int	i;
  for (i = 0; i < 0x30; i++)
    printf("%.2x", unk2[i]);
#if __linux__
  printf("\n      unk3 = 0x%.16lx, unk4 = %d\n", unk3, unk4);
#else
  printf("\n      unk3 = 0x%.16I64x, unk4 = %d\n", unk3, unk4);
#endif

  for (i = 0; i < 3; i++)
    {
      uint8_t	nb_frame_subentry = read_u8(fp, js, "nb_frame_subentry");
      if (nb_frame_subentry > 0)
	{
	  printf("Error: take_frame_subentries are not supported.\n");
	  exit(1);
	}
    }

  uint8_t	unk5 = read_u8(fp, js, "unk5");
  printf("      unk5 = %d\n", unk5);
}

static void	layer_element(FILE* fp, unsigned int entry_index, json_t *js)
{
  float		floats[16];
  floats[0]  = read_float(fp, js, "float_1");
  floats[1]  = intToFloat(read_u32(fp, js, "float_2"));
  floats[2]  = intToFloat(read_u32(fp, js, "float_3"));
  floats[3]  = intToFloat(read_u32(fp, js, "float_4"));
  floats[4]  = intToFloat(read_u32(fp, js, "float_5"));
  floats[5]  = read_float(fp, js, "float_6");
  floats[6]  = intToFloat(read_u32(fp, js, "float_7"));
  floats[7]  = intToFloat(read_u32(fp, js, "float_8"));
  floats[8]  = intToFloat(read_u32(fp, js, "float_9"));
  floats[9]  = intToFloat(read_u32(fp, js, "float_10"));
  floats[10] = read_float(fp, js, "float_11");
  floats[11] = intToFloat(read_u32(fp, js, "float_12"));
  floats[12] = read_float(fp, js, "float_x");
  floats[13] = read_float(fp, js, "float_y");
  floats[14] = intToFloat(read_u32(fp, js, "float_15"));
  floats[15] = read_float(fp, js, "float_16");
  uint32_t	argb = read_u32(fp, js, "argb");
  short	        name = read_u16(fp, js, "name");
  short		x = read_u16(fp, js, "x");
  short		y = read_u16(fp, js, "y");
  short		w = read_u16(fp, js, "w");
  short		h = read_u16(fp, js, "h");
  // Known names: argb, cx, cy, height, left, rx, ry, rz, sx, sy, top, width

  printf("      element %d:\n", entry_index + 1);
  unsigned int	i;
  for (i = 0; i < 0x10; i++)
    {
      printf("        argb[%d] = %.2f", i + 1, floats[i]);
      if (i + 1 == 13)
	printf(" (x)");
      else if (i + 1 == 14)
	printf(" (y)");
      printf("\n");
    }
  printf("        argb=%d, source=%s, x=%d, y=%d, w=%d, h=%d\n", argb, img_idx_to_name(name), x, y, w, h);
}

static void	layer(FILE* fp, unsigned int entry_index, json_t *js)
{
  uint8_t	unk1 = read_u8( fp, js, "unk1");
  uint32_t	unk2 = read_u32(fp, js, "unk2");
  uint8_t	unk3 = read_u8( fp, js, "unk3");
  uint8_t	unk4 = read_u8( fp, js, "unk4");
  uint8_t	unk5 = read_u8( fp, js, "unk5");
  uint8_t	unk6 = read_u8( fp, js, "unk6");
  uint16_t	unk7 = read_u16(fp, js, "unk7");

  printf("    layer %d: unk1 = %d, unk2 = %d, unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d, unk7 = %d\n", entry_index + 1, unk1, unk2, unk3, unk4, unk5, unk6, unk7);
  // Known names: blend, filter, flags, shader, type, nb_element

  unsigned int	i;
  for (i = 0; i < 1 /* TODO: find out the real value here */; i++)
    layer_element(fp, i, js_enter(js, idx_to_str("element_", i + 1)));
}

static void	take(FILE* fp, unsigned int entry_index, json_t *js)
{
  uint32_t	unk1 = read_u32(fp, js, "unk1");
  uint32_t	unk2 = read_u32(fp, js, "unk2");
  uint8_t	is_child = read_u8(fp, js, "is_child");
  printf("  take %d: unk1 = 0x%x, unk2 = 0x%x, is_child = %d\n", entry_index + 1, unk1, unk2, is_child);

  uint32_t	nb_frame = read_u32(fp, js, "nb_frame");
  unsigned int	i = 0;
  while (i < nb_frame)
    {
      frame(fp, i, js_enter(js, idx_to_str("frame_", i + 1)));
      i++;
    }

  uint8_t	nb_layer = read_u8(fp, js, "nb_layer");
  i = 0;
  while (i < nb_layer)
    {
      layer(fp, i, js_enter(js, idx_to_str("layer_", i + 1)));
      i++;
    }
}

void	take_array(FILE* fp, char* pat_path, json_t *js)
{
  (void)pat_path;
  uint32_t	nb_take = read_u32(fp, js, "nb_take");
  printf("take_array: %d take\n", nb_take);
  unsigned int	i = 0;
  while (i < nb_take)
    {
      take(fp, i, js_enter(js, idx_to_str("", i + 1)));
      i++;
    }
}
