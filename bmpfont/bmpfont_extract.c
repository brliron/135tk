#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct
{
  int16_t x;
  int16_t y;
  uint8_t width;
  uint8_t height;
  uint8_t y_offset;
  uint8_t channel;
} CharDetail;
#pragma pack(pop)

int main(int ac, const char** av)
{
  if (ac < 2)
    {
      printf("Usage: %s in.bmp [out.txt]\n", av[0]);
      return 0;
    }

  FILE *fin = fopen(av[1], "rb");
  if (!fin)
    {
      perror(av[1]);
      return 1;
    }
  FILE *fout;
  if (ac >= 3)
    fout = fopen(av[2], "w");
  else
    fout = stdout;

  BITMAPFILEHEADER header;
  fread(&header, sizeof(header), 1, fin);
  fseek(fin, header.bfSize, SEEK_SET);

  uint16_t unk;
  uint16_t nb_chars;
  fread(&unk,      2, 1, fin);
  fread(&nb_chars, 2, 1, fin);
  fprintf(fout, "unk: %d\n", unk);

  uint16_t *chars = malloc(nb_chars * sizeof(uint16_t));
  CharDetail *char_details = malloc(nb_chars * sizeof(CharDetail));
  fread(chars,        nb_chars * sizeof(uint16_t),   1, fin);
  fread(char_details, nb_chars * sizeof(CharDetail), 1, fin);

  uint16_t i;
  for (i = 0; i < nb_chars; i++)
    {
      const char* channel[] = {
	"blue",
	"green",
	"red",
	"alpha"
      };
      char str[3];
      if (chars[i] & 0xFF00)
	{
	  str[0] = chars[i] >> 8;
	  str[1] = chars[i] & 0xFF;
	  str[2] = 0;
	}
      else
	{
	  str[0] = chars[i];
	  str[1] = 0;
	}
      fprintf(fout, "%s: x=%d y=%d width=%d height=%d y_offset=%d channel=%s\n",
	      str,
	      char_details[i].x,
	      char_details[i].y,
	      char_details[i].width,
	      char_details[i].height,
	      char_details[i].y_offset,
	      channel[char_details[i].channel]
	      );
    }

  free(chars);
  free(char_details);
  fclose(fin);
  if (ac >= 3)
    fclose(fout);

  return 0;
}
