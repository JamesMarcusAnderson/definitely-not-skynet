#include "pmm.h"
#define PAGE_SIZE 4096
static uint8_t* pmm_bitmap = 0;
static uint64_t pmm_max_pages = 0;
static uint64_t pmm_total = 0;
static uint64_t pmm_free = 0;
static void set_bit(uint64_t i, int used) {
    uint64_t b = i / 8; uint8_t bit = i % 8;
    if (used) pmm_bitmap[b] |= (1 << bit); else pmm_bitmap[b] &= ~(1 << bit);
}
static int get_bit(uint64_t i) { return (pmm_bitmap[i / 8] >> (i % 8)) & 1; }
void pmm_init(EFI_MEMORY_DESCRIPTOR* map, uint64_t map_size, uint64_t desc_size, void* bitmap_memory, uint64_t bitmap_bytes, uint64_t max_phys) {
    pmm_bitmap = (uint8_t*)bitmap_memory;
    pmm_max_pages = max_phys / PAGE_SIZE;
    uint64_t entries = desc_size ? map_size / desc_size : 0;
    /* Mark all used first, bitmap_bytes must be ceil(max_pages/8). */
    for (uint64_t i = 0; i < bitmap_bytes; i++) pmm_bitmap[i] = 0xFF;
    pmm_total = 0;
    for (uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* d = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)map + (i * desc_size));
        if (d->Type != 7) continue; /* EfiConventionalMemory only */
        uint64_t start = d->PhysicalStart / PAGE_SIZE;
        for (uint64_t p = 0; p < d->NumberOfPages; p++) {
            uint64_t idx = start + p;
            if (idx >= pmm_max_pages) break;
            set_bit(idx, 0);
            pmm_total++;
        }
    }
    /* Reserve the bitmap itself, otherwise pmm_alloc_page could hand it out
       and overwrite the bitmap. Pasted draft used a fixed 1MB bitmap, which
       is too small for >8GB RAM (bitmap needs ceil(max_pages/8) bytes). */
    uint64_t bmp_start = ((uint64_t)bitmap_memory) / PAGE_SIZE;
    uint64_t bmp_pages = (bitmap_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint64_t p = 0; p < bmp_pages; p++) {
        uint64_t idx = bmp_start + p;
        if (idx < pmm_max_pages && !get_bit(idx)) { set_bit(idx, 1); pmm_total--; }
    }
    /* Count free pages after all marking, instead of trusting a counter
       that was decremented before bitmap reservation. */
    pmm_free = 0;
    for (uint64_t i = 0; i < pmm_max_pages; i++) if (!get_bit(i)) pmm_free++;
}
void* pmm_alloc_page(void) {
    for (uint64_t i = 0; i < pmm_max_pages; i++) {
        if (!get_bit(i)) { set_bit(i, 1); pmm_free--; return (void*)(i * PAGE_SIZE); }
    }
    return 0;
}
uint64_t pmm_total_pages(void) { return pmm_total; }
uint64_t pmm_free_pages(void) { return pmm_free; }
