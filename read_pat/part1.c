#include "read_pat.h"

void	part1(FILE* fp, char* pat_path, json_t *js)
{
  (void)pat_path;
  uint8_t	unk1 = read_u8(fp, js, "version");
  printf("part1:\n  version = %d\n", unk1);

  uint8_t	nb_entries = read_u8(fp, js, "nb_entries");
  unsigned int	i = 0;
  while (i < nb_entries)
    {
      uint8_t	use_entry = read_u8(fp, js, idx_to_str("use_entry_", i));
      int	entry = read_u32(fp, js, idx_to_str("entry_", i));
      printf("    %d: %d\n", i + 1, use_entry ? entry : -1);
      i++;
    }
}
