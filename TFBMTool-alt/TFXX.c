#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "TFBMTool.h"

FILE *TFXX_open(const char *fn, const char *in_magic, void *header, size_t header_size)
{
  FILE *f = fopen(fn, "rb");
  if (f == NULL) {
    perror(fn);
    return NULL;
  }

  char magic[4];
  uint8_t version;
  fread(magic, 4, 1, f);
  fread(&version, 1, 1, f);
  if (memcmp(in_magic, magic, 4) != 0 || version != 0) {
    printf("Error: %s: wrong magic or version\n", fn);
    fclose(f);
    return NULL;
  }

  fread(header, header_size, 1, f);

  return f;
}

static int inflate_bytes(char* file_in, size_t size_in, char* file_out, size_t size_out)
{
	int ret;
	z_stream strm;

	strm.zalloc = NULL;
	strm.zfree = NULL;
	strm.opaque = NULL;
	strm.next_in = (unsigned char*)file_in;
	strm.avail_in = size_in;
	strm.next_out = (unsigned char*)file_out;
	strm.avail_out = size_out;
	ret = inflateInit(&strm);
	if (ret != Z_OK) {
		return ret;
	}
	do {
		ret = inflate(&strm, Z_NO_FLUSH);
		if (ret != Z_OK) {
			break;
		}
	} while (ret != Z_STREAM_END);
	inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : ret;
}

char *TFXX_read(FILE *f, size_t comp_size, size_t uncomp_size)
{
  if (!f) {
    return NULL;
  }

  char *comp_data = malloc(comp_size);
  fread(comp_data, comp_size, 1, f);
  fclose(f);

  char *uncomp_data = malloc(uncomp_size);
  if (inflate_bytes(comp_data, comp_size, uncomp_data, uncomp_size) != Z_OK) {
    printf("inflate error\n");
    free(uncomp_data);
    uncomp_data = NULL;
  }
  free(comp_data);

  return uncomp_data;
}
