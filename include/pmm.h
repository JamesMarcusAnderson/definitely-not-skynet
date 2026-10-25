#pragma once
#include <stdint.h>
#include "uefi.h"
void pmm_init(EFI_MEMORY_DESCRIPTOR* map, uint64_t map_size, uint64_t desc_size, void* bitmap_memory, uint64_t bitmap_bytes, uint64_t max_phys);
void* pmm_alloc_page(void);
uint64_t pmm_total_pages(void);
uint64_t pmm_free_pages(void);
