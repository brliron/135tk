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

static void     frame_subentry1_child(FILE* fp, unsigned int entry_index, json_t* js)
{
  uint8_t	unk1     = read_u8(fp, js, "unk1");
  float		width    = read_float(fp, js, "width");
  float		height   = read_float(fp, js, "height");
  float		x_offset = read_float(fp, js, "x_offset");
  float		y_offset = read_float(fp, js, "y_offset");
  uint16_t	unk6     = read_u16(fp, js, "unk6"); // Converted to a uint32_t, then loaded in xmm0 with movd.
  printf("        child %d: unk1 = %d, width = %.2f, height = %.2f, x_offset = %.2f, y_offset = %.2f, unk6 = %.2x\n",
	 entry_index + 1, unk1, width, height, x_offset, y_offset, unk6);
}

static void     box_data(FILE* fp, const char *name, json_t* js)
{
  uint8_t	nb_child = read_u8(fp, js, "nb_child");
  printf("      %s: %d childs\n", name, nb_child);
  unsigned int	i;
  for (i = 0; i < nb_child; i++)
    frame_subentry1_child(fp, i, js_enter(js, idx_to_str("child_", i + 1)));
}

static void     projectile(FILE* fp, unsigned int entry_index, json_t* js)
{
  uint64_t	value = read_u64(fp, js, "value");

  printf("        projectile %d: ", entry_index + 1);
#if __linux__
  printf("value = 0x%.16lx\n", value);
#else
  printf("value = 0x%.16llx\n", value);
#endif
}

static void     frame(FILE* fp, unsigned int entry_index, json_t* js)
{
  uint16_t	unk1        = read_u16(fp, js, "unk1");
  int16_t	damagePoint = read_i16(fp, js, "damagePoint");
  int16_t	hitStopE    = read_i16(fp, js, "hitStopE");
  int16_t	hitStopP    = read_i16(fp, js, "hitStopP");
  int16_t	guardStopE  = read_i16(fp, js, "guardStopE");
  int16_t	guardStopP  = read_i16(fp, js, "guardStopP");
  int16_t	firstRate   = read_i16(fp, js, "firstRate");
  int16_t	comboRate   = read_i16(fp, js, "comboRate");
  int16_t	addKnock    = read_i16(fp, js, "addKnock");
  int16_t	addStan     = read_i16(fp, js, "addStan");
  int16_t	bariaBreak  = read_i16(fp, js, "bariaBreak");
  int16_t	guardDamage = read_i16(fp, js, "guardDamage");
  int16_t	guardLost   = read_i16(fp, js, "guardLost");
  int16_t	addSP       = read_i16(fp, js, "addSP");
  int16_t	recover     = read_i16(fp, js, "recover");
  int16_t	minRate     = read_i16(fp, js, "minRate");
  int16_t	stopVecX    = read_i16(fp, js, "stopVecX");
  int16_t	stopVecY    = read_i16(fp, js, "stopVecY");
  int16_t	hitSE       = read_i16(fp, js, "hitSE");
  int16_t	hitVecX     = read_i16(fp, js, "hitVecX");
  int16_t	hitVecY     = read_i16(fp, js, "hitVecY");
  int16_t	grazeKnock  = read_i16(fp, js, "grazeKnock");
  int16_t	atkType     = read_i16(fp, js, "atkType");
  int16_t	atkRank     = read_i16(fp, js, "atkRank");
  int16_t	hitEffect   = read_i16(fp, js, "hitEffect");

  printf("    frame %d:\n"
         "      unk1 = %d\n"
         "      damagePoint = %d\n"
         "      hitStopE = %d\n"
         "      hitStopP = %d\n"
         "      guardStopE = %d\n"
         "      guardStopP = %d\n"
         "      firstRate = %d\n"
         "      comboRate = %d\n"
         "      addKnock = %d\n"
         "      addStan = %d\n"
         "      bariaBreak = %d\n"
         "      guardDamage = %d\n"
         "      guardLost = %d\n"
         "      addSP = %d\n"
         "      recover = %d\n"
         "      minRate = %d\n"
         "      stopVecX = %d\n"
         "      stopVecY = %d\n"
         "      hitSE = %d\n"
         "      hitVecX = %d\n"
         "      hitVecY = %d\n"
         "      grazeKnock = %d\n"
         "      atkType = %d\n"
         "      atkRank = %d\n"
         "      hitEffect = %d\n",
    entry_index + 1,
    unk1,
    damagePoint,
    hitStopE,
    hitStopP,
    guardStopE,
    guardStopP,
    firstRate,
    comboRate,
    addKnock,
    addStan,
    bariaBreak,
    guardDamage,
    guardLost,
    addSP,
    recover,
    minRate,
    stopVecX,
    stopVecY,
    hitSE,
    hitVecX,
    hitVecY,
    grazeKnock,
    atkType,
    atkRank,
    hitEffect
  );


  uint64_t	flags = read_u64(fp, js, "flags");
  uint8_t	boxcount = read_u8(fp, js, "boxcount");

#if __linux__
  printf("      flags = 0x%.16lx\n", flags);
#else
  printf("      flags = 0x%.16llx\n", flags);
#endif
  printf("      boxcount = %d\n", boxcount);

  box_data(fp, "collisionbox_data", js_enter(js, "collisionbox_data"));
  box_data(fp, "hurtbox_data",      js_enter(js, "hurtbox_data"));
  box_data(fp, "hitbox_data",       js_enter(js, "hitbox_data"));

  uint8_t	nb_projectiles = read_u8(fp, js, "nb_projectiles");
  for (int i = 0; i < nb_projectiles; i++)
    projectile(fp, i, js_enter(js, idx_to_str("projectile_", i + 1)));
}

static void	matrix(FILE* fp, const char *padding, json_t *js)
{
  float		floats[16];
  floats[0]  = read_float(fp, js, "matrix[0][0]");
  floats[1]  = read_float(fp, js, "matrix[0][1]");
  floats[2]  = read_float(fp, js, "matrix[0][2]");
  floats[3]  = read_float(fp, js, "matrix[0][3]");
  floats[4]  = read_float(fp, js, "matrix[1][0]");
  floats[5]  = read_float(fp, js, "matrix[1][1]");
  floats[6]  = read_float(fp, js, "matrix[1][2]");
  floats[7]  = read_float(fp, js, "matrix[1][3]");
  floats[8]  = read_float(fp, js, "matrix[2][0]");
  floats[9]  = read_float(fp, js, "matrix[2][1]");
  floats[10] = read_float(fp, js, "matrix[2][2]");
  floats[11] = read_float(fp, js, "matrix[2][3]");
  floats[12] = read_float(fp, js, "matrix[3][0]");
  floats[13] = read_float(fp, js, "matrix[3][1]");
  floats[14] = read_float(fp, js, "matrix[3][2]");
  floats[15] = read_float(fp, js, "matrix[3][3]");
  // Known names: cx, cy, rx, ry, rz, sx, sy

  printf("%smatrix:\n", padding);
  unsigned int	i;
  unsigned int	j;
  for (i = 0; i < 4; i++)
    {
      printf("%s  [ ", padding);
      for (j = 0; j < 4; j++)
	{
	  printf("%.2f", floats[i * 4 + j]);
	  if (j + 1 < 4)
	    printf(",");
	  printf(" ");
	}
      printf("]\n");
    }
}

static void	layer_type0_element(FILE* fp, unsigned int entry_index, json_t *js)
{
  printf("      element %d:\n", entry_index + 1);
  matrix(fp, "        ", js);

  uint32_t	argb = read_u32(fp, js, "argb");
  short	        name = read_u16(fp, js, "name");
  short		x = read_u16(fp, js, "x"); // left
  short		y = read_u16(fp, js, "y"); // top
  short		w = read_u16(fp, js, "w"); // width
  short		h = read_u16(fp, js, "h"); // height

  printf("        argb=%.8X, source=%s, x=%d, y=%d, w=%d, h=%d\n", argb, img_idx_to_name(name), x, y, w, h);
}

static void	layer_type0(FILE* fp, json_t *js)
{
  uint8_t	unk3 = read_u8( fp, js, "unk3");
  uint8_t	unk4 = read_u8( fp, js, "unk4");
  uint8_t	unk5 = read_u8( fp, js, "unk5");
  uint8_t	unk6 = read_u8( fp, js, "unk6");
  uint16_t	nb_element = read_u16(fp, js, "nb_element");

  printf(", unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d\n", unk3, unk4, unk5, unk6);
  // Known names: blend, filter, flags, shader

  unsigned int	i;
  for (i = 0; i < nb_element; i++)
    layer_type0_element(fp, i, js_enter(js, idx_to_str("element_", i + 1)));
}

static void	layer_type1_element(FILE* fp, unsigned int entry_index, json_t *js)
{
  printf("      element %d:\n", entry_index + 1);
  matrix(fp, "        ", js);

  uint32_t	argb = read_u32(fp, js, "argb");
  uint16_t	unk1 = read_u16(fp, js, "unk1");
  uint16_t	unk2 = read_u16(fp, js, "unk2");
  uint16_t	unk3 = read_u16(fp, js, "unk3");
  uint16_t	unk4 = read_u16(fp, js, "unk4");
  uint16_t	unk5 = read_u16(fp, js, "unk5");
  uint16_t	unk6 = read_u16(fp, js, "unk6");
  uint16_t	unk7 = read_u16(fp, js, "unk7");
  uint16_t	unk8 = read_u16(fp, js, "unk8");

  printf("        argb=%.8X, unk1 = %d, unk2 = %d, unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d, unk7 = %d, unk8 = %d\n",
	 argb, unk1, unk2, unk3, unk4, unk5, unk6, unk7, unk8);
}

static void	layer_type1(FILE* fp, json_t *js)
{
  uint8_t	unk3 = read_u8( fp, js, "unk3");
  uint8_t	unk4 = read_u8( fp, js, "unk4");
  uint8_t	unk5 = read_u8( fp, js, "unk5");
  uint8_t	unk6 = read_u8( fp, js, "unk6");
  uint16_t	nb_element = read_u16(fp, js, "nb_element");

  printf(", unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d\n", unk3, unk4, unk5, unk6);
  // Known names: blend, filter, flags, shader

  unsigned int	i;
  for (i = 0; i < nb_element; i++)
    layer_type1_element(fp, i, js_enter(js, idx_to_str("element_", i + 1)));
}

static void	layer_type2_element(FILE* fp, unsigned int entry_index, json_t *js)
{
  printf("      element %d:\n", entry_index + 1);
  matrix(fp, "        ", js);

  uint32_t	argb = read_u32(fp, js, "argb");
  uint16_t	unk1 = read_u16(fp, js, "unk1");
  uint16_t	unk2 = read_u16(fp, js, "unk2");
  uint16_t	unk3 = read_u16(fp, js, "unk3");
  uint16_t	unk4 = read_u16(fp, js, "unk4");
  uint16_t	unk5 = read_u16(fp, js, "unk5");
  uint16_t	unk6 = read_u16(fp, js, "unk6");
  uint16_t	unk7 = read_u16(fp, js, "unk7");
  uint16_t	unk8 = read_u16(fp, js, "unk8");
  uint16_t	unk9 = read_u16(fp, js, "unk9");

  printf("        argb=%.8X, unk1 = %d, unk2 = %d, unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d, unk7 = %d, unk8 = %d, unk9 = %d\n",
	 argb, unk1, unk2, unk3, unk4, unk5, unk6, unk7, unk8, unk9);
}

static void	layer_type2(FILE* fp, json_t *js)
{
  uint8_t	unk3 = read_u8( fp, js, "unk3");
  uint8_t	unk4 = read_u8( fp, js, "unk4");
  uint8_t	unk5 = read_u8( fp, js, "unk5");
  uint8_t	unk6 = read_u8( fp, js, "unk6");
  uint16_t	nb_element = read_u16(fp, js, "nb_element");

  printf(", unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d\n", unk3, unk4, unk5, unk6);
  // Known names: blend, filter, flags, shader

  unsigned int	i;
  for (i = 0; i < nb_element; i++)
    layer_type2_element(fp, i, js_enter(js, idx_to_str("element_", i + 1)));
}

static void	layer_type3_element(FILE* fp, unsigned int entry_index, json_t *js)
{
  printf("      element %d:\n", entry_index + 1);
  matrix(fp, "        ", js);

  uint32_t	argb = read_u32(fp, js, "argb");
  uint16_t	unk1 = read_u16(fp, js, "unk1");
  uint16_t	unk2 = read_u16(fp, js, "unk2");
  uint16_t	unk3 = read_u16(fp, js, "unk3");
  uint16_t	unk4 = read_u16(fp, js, "unk4");
  uint16_t	unk5 = read_u16(fp, js, "unk5");
  uint16_t	unk6 = read_u16(fp, js, "unk6");
  uint16_t	unk7 = read_u16(fp, js, "unk7");

  printf("        argb=%.8X, unk1 = %d, unk2 = %d, unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d, unk7 = %d\n",
	 argb, unk1, unk2, unk3, unk4, unk5, unk6, unk7);
}

static void	layer_type3(FILE* fp, json_t *js)
{
  uint8_t	unk3 = read_u8( fp, js, "unk3");
  uint8_t	unk4 = read_u8( fp, js, "unk4");
  uint8_t	unk5 = read_u8( fp, js, "unk5");
  uint8_t	unk6 = read_u8( fp, js, "unk6");
  uint16_t	nb_element = read_u16(fp, js, "nb_element");

  printf(", unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d\n", unk3, unk4, unk5, unk6);
  // Known names: blend, filter, flags, shader

  unsigned int	i;
  for (i = 0; i < nb_element; i++)
    layer_type3_element(fp, i, js_enter(js, idx_to_str("element_", i + 1)));
}

static void	layer_type6(FILE* fp, json_t *js)
{
  uint8_t	unk3 = read_u8( fp, js, "unk3");
  uint8_t	unk4 = read_u8( fp, js, "unk4");
  uint8_t	unk5 = read_u8( fp, js, "unk5");
  uint8_t	unk6 = read_u8( fp, js, "unk6");

  printf(", unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d\n", unk3, unk4, unk5, unk6);
  // Known names: blend, filter, flags, shader

  matrix(fp, "      ", js);

  uint32_t	eft_size = read_u32(fp, js, "eft_size");
  char		eft[256];

  read_str(fp, eft, eft_size, js, "eft");
  eft[eft_size] = '\0';
  printf("      eft=%s, ", eft);

  uint16_t	unk7 = read_u16(fp, js, "unk7");
  uint16_t	unk8 = read_u16(fp, js, "unk8");

  printf("unk7 = %d, unk8 = %d\n", unk7, unk8);
}

static void	layer_type7_element(FILE* fp, unsigned int entry_index, json_t *js)
{
  printf("      element %d:\n", entry_index + 1);

  uint32_t	argb = read_u32(fp, js, "argb");
  short	        name = read_u16(fp, js, "name");
  short		x = read_u16(fp, js, "x"); // left
  short		y = read_u16(fp, js, "y"); // top
  short		w = read_u16(fp, js, "w"); // width
  short		h = read_u16(fp, js, "h"); // height

  printf("        argb=%.8X, source=%s, x=%d, y=%d, w=%d, h=%d\n", argb, img_idx_to_name(name), x, y, w, h);
}

static void	layer_type7(FILE* fp, json_t *js)
{
  uint8_t	unk3 = read_u8( fp, js, "unk3");
  uint8_t	unk4 = read_u8( fp, js, "unk4");
  uint8_t	unk5 = read_u8( fp, js, "unk5");
  uint8_t	unk6 = read_u8( fp, js, "unk6");
  uint16_t	unk7 = read_u16(fp, js, "unk7");
  uint16_t	unk8 = read_u16(fp, js, "unk8");
  uint16_t	nb_element = read_u16(fp, js, "nb_element");

  printf(", unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d, unk7 = %d, unk8 = %d\n", unk3, unk4, unk5, unk6, unk7, unk8);

  unsigned int	i;
  for (i = 0; i < nb_element; i++)
    layer_type7_element(fp, i, js_enter(js, idx_to_str("element_", i + 1)));
}

static void	layer_type128_element(FILE* fp, unsigned int entry_index, json_t *js)
{
  printf("      element %d:\n", entry_index + 1);
  matrix(fp, "        ", js);

  uint32_t	argb = read_u32(fp, js, "argb");
  short	        name = read_u16(fp, js, "name");
  short		x = read_u16(fp, js, "x"); // left
  short		y = read_u16(fp, js, "y"); // top
  short		w = read_u16(fp, js, "w"); // width
  short		h = read_u16(fp, js, "h"); // height

  printf("        argb=%.8X, source=%s, x=%d, y=%d, w=%d, h=%d\n", argb, img_idx_to_name(name), x, y, w, h);
}

static void	layer_type128(FILE* fp, json_t *js)
{
  uint8_t	unk3 = read_u8( fp, js, "unk3");
  uint8_t	unk4 = read_u8( fp, js, "unk4");
  uint8_t	unk5 = read_u8( fp, js, "unk5");
  uint8_t	unk6 = read_u8( fp, js, "unk6");
  uint16_t	nb_element = read_u16(fp, js, "nb_element");

  printf(", unk3 = %d, unk4 = %d, unk5 = %d, unk6 = %d\n", unk3, unk4, unk5, unk6);
  // Known names: blend, filter, flags, shader

  unsigned int	i;
  for (i = 0; i < nb_element; i++)
    layer_type128_element(fp, i, js_enter(js, idx_to_str("element_", i + 1)));
}

static void	layer(FILE* fp, unsigned int entry_index, json_t *js)
{
  uint8_t	type = read_u8( fp, js, "type");
  uint32_t	unk2 = read_u32(fp, js, "unk2");

  printf("    layer %d: type = %d, unk2 = %d", entry_index + 1, type, unk2);

  switch (type)
    {
    case 0:
      layer_type0(fp, js);
      break;
    case 1:
      layer_type1(fp, js);
      break;
    case 2:
      layer_type2(fp, js);
      break;
    case 3:
      layer_type3(fp, js);
      break;
    case 6:
      layer_type6(fp, js);
      break;
    case 7:
      layer_type7(fp, js);
      break;
    case 128:
      layer_type128(fp, js);
      break;
    default:
      printf("\n    Error: unknown layer type %d\n", type);
      exit(1);
    }
}

static void	take(FILE* fp, unsigned int entry_index, json_t *js)
{
  int32_t	motion_id = read_i32(fp, js, "motion_id");
  uint32_t	unk2 = read_u32(fp, js, "unk2");
  uint8_t	is_child = read_u8(fp, js, "is_child");
  printf("  take %d: motion_id = 0x%x, unk2 = 0x%x, is_child = %d\n", entry_index + 1, motion_id, unk2, is_child);

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
