#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int ac, const char** av)
{
  if (ac != 3 || (av[1][0] != '/' && av[1][0] != '-'))
    {
      printf("Usage: %s [/x|/p] file.nthex\n", av[0]);
      return 0;
    }

  FILE *fin = fopen(av[2], "rb");
  if (!fin)
    {
      perror(av[2]);
      return 1;
    }

  uint32_t header[12];
  fseek(fin, 0, SEEK_END);
  size_t size = ftell(fin);
  fseek(fin, 0, SEEK_SET);
  fread(&header, 0x30, 1, fin);
  if (header[6] + 0x30 != size)
    printf("Warning: file size and header size don't match.\n");
  uint32_t expected_header[12] = {
    0x20, 0, 0x10, 0,
    0x20, 0, header[6], 0,
    0, 0, 0, 0
  };
  if (memcmp(header, expected_header, 0x30) != 0)
    printf("Warning: the file header doesn't match the expected one.\n");

  char *data = malloc(header[6]);
  fread(data, header[6], 1, fin);
  fclose(fin);

  char out_fn[260];
  strcpy(out_fn, av[2]);
  if (memcmp(data, "\x89PNG", 4) == 0)
    strcat(out_fn, ".png");
  else if (memcmp(data, "DDS ", 4) == 0)
    strcat(out_fn, ".dds");
  else
    strcat(out_fn, ".out");

  if (av[1][1] == 'x')
    {
      FILE *fout = fopen(out_fn, "wb");
      fwrite(data, header[6], 1, fout);
      fclose(fout);
    }
  else if (av[1][1] == 'p')
    {
      free(data);
      FILE *fin2 = fopen(out_fn, "rb");
      fseek(fin2, 0, SEEK_END);
      size_t size_in2 = ftell(fin2);
      fseek(fin2, 0, SEEK_SET);
      data = malloc(size_in2);
      fread(data, size_in2, 1, fin2);
      fclose(fin2);

      FILE *fout = fopen(av[2], "wb");
      header[6] = size_in2;
      fwrite(header, 0x30,     1, fout);
      fwrite(data,   size_in2, 1, fout);
      fclose(fout);
    }
  else
    printf("Error: unknown parameter %s\n", av[1]);

  free(data);
  return 0;
}
