#include "read_pat.h"

void	part4(FILE* fp, char* pat_path, json_t *js)
{
  (void)pat_path;
  uint32_t	nb_entries = read_u32(fp, js, "nb_entries");
  printf("part4: nb_entries = %d\n", nb_entries);
  if (nb_entries > 0)
    printf("Error: part4_entries are not supported.\n");
}
