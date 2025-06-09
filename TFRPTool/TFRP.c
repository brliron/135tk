#include <stdio.h>
#include <stdlib.h>
#include "TFXX.h"
#include "TFRP.h"

int main(int ac, char **av)
{
  (void)ac;

  TFRP_header header;
  FILE *f = TFXX_open_read(av[1], "TFRP", &header, sizeof(header));

  char *metadata = TFXX_read(f, header.metadata_size_compressed, header.metadata_size_uncompressed);
  if (!metadata) {
    return 1;
  }

  FILE *out = fopen("1_uncompressed_metadata.bin", "wb");
  if (out) {
    fwrite(metadata, header.metadata_size_uncompressed, 1, out);
  }
  free(metadata);
  fclose(out);

  struct {
    uint32_t replay_type; // 1 = macro, 2 = match replay
    uint32_t compressed_size;
    uint32_t uncompressed_size;
  } second_header;
  fread(&second_header, 12, 1, f);

  char *data = TFXX_read(f, second_header.compressed_size, second_header.uncompressed_size);
  if (!data) {
    printf("2nd read failed\n");
    return 1;
  }

  out = fopen("2_uncompressed_data.bin", "wb");
  if (out) {
    fwrite(data, second_header.uncompressed_size, 1, out);
  }
  free(data);
  fclose(out);

  if (second_header.replay_type == 2) {
    uint32_t unknown_block_size;
    fread(&unknown_block_size, 4, 1, f);
    char *unknown_block = malloc(unknown_block_size);
    fread(unknown_block, unknown_block_size, 1, f);

    out = fopen("3_unknown_block.bin", "wb");
    if (out) {
      fwrite(unknown_block, unknown_block_size, 1, out);
    }
    free(unknown_block);
    fclose(out);

    uint32_t unknown_uint = 0xFFFFFFFF;
    fread(&unknown_uint, 4, 1, f);
    printf("Unkonwn uint: %x\n", unknown_uint);
  }
  else if (second_header.replay_type == 1) {
    // Nothing to do
  }
  else {
    fprintf(stderr, "Unknown replay type %d\n", second_header.replay_type);
  }

  return 0;
}
