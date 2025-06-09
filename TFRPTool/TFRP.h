#pragma once

#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
  uint32_t unk1;
  uint32_t unk2;
  uint32_t metadata_size_compressed;
  uint32_t metadata_size_uncompressed;
} TFRP_header;
