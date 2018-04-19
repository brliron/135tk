#include "read_pat.h"

static void	part4_entry(FILE* fp, unsigned int entry_index, json_t *js)
{
  uint32_t	unk1 = read_u32(fp, js, "unk1");
  uint32_t	unk2 = read_u32(fp, js, "unk2");
  printf("  entry %d: unk1=%d, unk2=%d\n", entry_index, unk1, unk2);
}

void	part4(FILE* fp, char* pat_path, json_t *js)
{
  (void)pat_path;
  uint32_t	nb_entries = read_u32(fp, js, "nb_entries");
  printf("part4: nb_entries = %d\n", nb_entries);

  unsigned int	i;
  for (i = 0; i < nb_entries; i++)
    part4_entry(fp, i, js_enter(js, idx_to_str("entry_", i)));
}
