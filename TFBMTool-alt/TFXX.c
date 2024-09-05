#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "TFBMTool.h"

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
			fwprintf(stderr, L"Warning: zlib end of stream not found.\n");
			// Some streams are just missing the end of stream marker.
			// To accommodate for these streams, we'll pretend everything worked.
			ret = Z_STREAM_END;
		}
		else if (strm.avail_out == 0) {
			fwprintf(stderr, L"Decompression error: invalid TFBM header.\n");
		}
		else {
			fwprintf(stderr, L"Decompression error %d\n", ret);
		}
	}
	else {
		fwprintf(stderr, L"Decompression error %d\n", ret);
	}
	return ret;
}

FILE *TFXX_open_read(LPCWSTR fn, const char *in_magic, void *header, size_t header_size)
{
  FILE *f = _wfopen(fn, L"rb");
  if (f == NULL) {
    _wperror(fn);
    return NULL;
  }

  char magic[4];
  uint8_t version;
  fread(magic, 4, 1, f);
  fread(&version, 1, 1, f);
  if (memcmp(in_magic, magic, 4) != 0 || version != 0) {
    fwprintf(stderr, L"Error: %S: wrong magic or version\n", fn);
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
  fclose(f);

  char *uncomp_data = malloc(uncomp_size);
  if (inflate_bytes(comp_data, comp_size, uncomp_data, uncomp_size) != Z_STREAM_END) {
    free(uncomp_data);
    uncomp_data = NULL;
  }
  free(comp_data);

  return uncomp_data;
}

/*
static int deflate_bytes_to_file(const char *file_in, size_t size_in, FILE *f_out)
{
  int ret;
  z_stream strm;
  unsigned char buffer[4096];

  strm.zalloc = NULL;
  strm.zfree = NULL;
  strm.opaque = NULL;
  strm.next_in = (unsigned char*)file_in;
  strm.avail_in = size_in;
  strm.next_out = buffer;
  strm.avail_out = 4096;
  ret = deflateInit(&strm, Z_BEST_COMPRESSION);
  if (ret != Z_OK) {
    return ret;
  }
  do {
    ret = deflate(&strm, Z_FINISH);
    size_t consumed = 4096 - strm.avail_out;
    if (consumed > 0) {
      fwrite(buffer, consumed, 1, f_out);
    }
    strm.next_out = buffer;
    strm.avail_out = 4096;
    if (ret != Z_OK) {
      break;
    }
  } while (ret != Z_STREAM_END);
  deflateEnd(&strm);
  return ret == Z_STREAM_END ? Z_OK : ret;
}
*/

static int deflate_bytes(const char* file_in, size_t size_in, char* file_out, uint32_t* size_out)
{
	int ret;
	z_stream strm;

	strm.zalloc = NULL;
	strm.zfree = NULL;
	strm.opaque = NULL;
	strm.next_in = (unsigned char*)file_in;
	strm.avail_in = size_in;
	strm.next_out = (unsigned char*)file_out;
	strm.avail_out = *size_out;
	ret = deflateInit(&strm, Z_BEST_COMPRESSION);
	if (ret != Z_OK) {
		return ret;
	}
	do {
		ret = deflate(&strm, Z_FINISH);
		if (ret != Z_OK) {
			break;
		}
	} while (ret != Z_STREAM_END);
	deflateEnd(&strm);
	*size_out -= strm.avail_out;
	return ret == Z_STREAM_END ? Z_OK : ret;
}

FILE *TFXX_open_write(LPCWSTR fn, const char *magic, const void *header, size_t header_size)
{
  FILE *f = _wfopen(fn, L"wb");
  if (f == NULL) {
    _wperror(fn);
    return NULL;
  }

  uint8_t version = 0;
  fwrite(magic, 4, 1, f);
  fwrite(&version, 1, 1, f);
  if (header_size > 4)
    fwrite(header, header_size - 4 /* remove comp_size */, 1, f);

  return f;
}

/*
void TFXX_write(FILE *f, const char *uncomp_data, size_t uncomp_size)
{
  if (!f) {
    return;
  }

  if (deflate_bytes_to_file(uncomp_data, uncomp_size, f) != Z_OK) {
    printf("deflate error\n");
  }
  fclose(f);
}
*/

void TFXX_write(FILE *f, const char *uncomp_data, size_t uncomp_size)
{
  if (!f) {
    return;
  }

  uint32_t comp_size = uncomp_size + 512;
  char *comp_data = malloc(comp_size);
  if (deflate_bytes(uncomp_data, uncomp_size, comp_data, &comp_size) != Z_OK) {
    fwprintf(stderr, L"deflate error\n");
  }
  fwrite(&comp_size, 4, 1, f);
  fwrite(comp_data, comp_size, 1, f);
  fclose(f);
}
