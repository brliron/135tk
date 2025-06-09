#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

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
	ret = inflate(&strm, Z_FINISH);
	inflateEnd(&strm);

	if (ret == Z_STREAM_END) {
		// Stream finished - everything is good.
	}
	else if (ret == Z_OK || ret == Z_BUF_ERROR) {
		if (strm.avail_in == 0) {
			fprintf(stderr, "Warning: zlib end of stream not found.\n");
			// Some streams are just missing the end of stream marker.
			// To accommodate for these streams, we'll pretend everything worked.
			ret = Z_STREAM_END;
		}
		else if (strm.avail_out == 0) {
			fprintf(stderr, "Decompression error: wrong size in header.\n");
		}
		else {
			fprintf(stderr, "Decompression error %d\n", ret);
		}
	}
	else {
		fprintf(stderr, "Decompression error %d\n", ret);
	}
	return ret;
}

FILE *TFXX_open_read(char* fn, const char *in_magic, void *header, size_t header_size)
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
  if (memcmp(magic, in_magic, 4) != 0) {
    fprintf(stderr, "Error: %s: wrong file format\n", fn);
    fclose(f);
    return NULL;
  }

  if (version != 0) {
    fprintf(stderr, "Error: %s: wrong version number in header\n", fn);
    fclose(f);
    return NULL;
  }

  fread(header, header_size, 1, f);

  return f;
}

char *TFXX_read(FILE *f, size_t comp_size, size_t uncomp_size)
{
  if (!f) {
    return NULL;
  }

  char *comp_data = malloc(comp_size);
  fread(comp_data, comp_size, 1, f);
  // fclose(f);

  char *uncomp_data = malloc(uncomp_size);
  memset(uncomp_data, 0xAB, uncomp_size);
  if (inflate_bytes(comp_data, comp_size, uncomp_data, uncomp_size) != Z_STREAM_END) {
    free(uncomp_data);
    uncomp_data = NULL;
  }
  free(comp_data);

  return uncomp_data;
}
