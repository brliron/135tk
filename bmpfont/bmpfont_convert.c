#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

int main(int ac, const char** av)
{
  if (ac < 2)
    {
      printf("Usage: %s in.bmp\n", av[0]);
      return 0;
    }

  FILE *fin = fopen(av[1], "rb");
  if (!fin)
    {
      perror(av[1]);
      return 1;
    }
  char *path_bin = (char*)malloc(strlen(av[1]) + 5);
  strcpy(path_bin, av[1]);
  strcat(path_bin, ".bin");
  FILE *fout = fopen(path_bin, "wb");
  if (fout == NULL)
    perror(path_bin);
  free(path_bin);

  BITMAPFILEHEADER header;
  fread(&header, sizeof(header), 1, fin);
  fseek(fin, header.bfSize, SEEK_SET);

  uint16_t unk;
  uint16_t nb_chars;
  fread(&unk,       2, 1, fin);
  fwrite(&unk,      2, 1, fout);
  fread(&nb_chars,  2, 1, fin);
  fwrite(&nb_chars, 2, 1, fout);

  char *chars = malloc(nb_chars * sizeof(uint16_t));
  char *char_details = malloc(nb_chars * 8);
  fread(chars,        nb_chars * sizeof(uint16_t), 1, fin);
  fread(char_details, nb_chars * 8,                1, fin);

  uint16_t i;
  for (i = 0; i < nb_chars; i++)
    {
      char cstr[2];
      WCHAR wstr[3];
      if (chars[i * 2 + 1] != 0)
	{
	  cstr[0] = chars[i * 2 + 1];
	  cstr[1] = chars[i * 2];
	}
      else
	{
	  cstr[0] = chars[i * 2];
	  cstr[1] = 0;
	}
      MultiByteToWideChar(932, 0, cstr, 2, wstr, 3);
      fwrite(wstr, 2, 1, fout);
    }
  fwrite(char_details, nb_chars * 8, 1, fout);

  free(chars);
  free(char_details);
  fclose(fin);
  fclose(fout);

  return 0;
}
