#include <stdint.h>
#include "uefi.h"
#include "pmm.h"
extern void serial_init(void);
extern void serial_write_string(const char* str);
extern void pcie_scan(void);
static void uefi_print(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* con, const uint16_t* s) { if (con && con->OutputString) con->OutputString(con, s); }
void kmain(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* sys_table) {
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* con = sys_table->ConOut;
    uefi_print(con, (const uint16_t*)u"\nDefinitely Not Skynet: bare-metal Phase 1.2\r\n");
    serial_init();
    serial_write_string("\n=== DEFINITELY NOT SKYNET: BARE METAL ===\n");
    EFI_BOOT_SERVICES* bs = sys_table->BootServices;
    uint64_t map_size = 0, map_key = 0, desc_size = 0; uint32_t desc_ver = 0;
    EFI_STATUS status;
    /* First call: get size. Real UEFI order is (size, map, key, desc_size, desc_ver).
       Pasted DeepSeek header swapped size and map, which would break this call. */
    status = bs->GetMemoryMap(&map_size, 0, &map_key, &desc_size, &desc_ver);
    if (status != EFI_BUFFER_TOO_SMALL && status != EFI_SUCCESS) { serial_write_string("[FAIL] GetMemoryMap size query failed.\n"); return; }
    map_size += 4096;
    void* mem_map = 0;
    /* PoolType 2 = EfiLoaderData, which survives ExitBootServices. */
    status = bs->AllocatePool(2, map_size, &mem_map);
    if (status != EFI_SUCCESS || !mem_map) { serial_write_string("[FAIL] AllocatePool for memory map failed.\n"); return; }
    status = bs->GetMemoryMap(&map_size, (EFI_MEMORY_DESCRIPTOR*)mem_map, &map_key, &desc_size, &desc_ver);
    if (status != EFI_SUCCESS) { serial_write_string("[FAIL] Could not get memory map.\n"); return; }
    uefi_print(con, (const uint16_t*)u"[OK] UEFI Memory Map acquired\r\n");
    serial_write_string("[OK] UEFI Memory Map acquired.\n");
    /* Compute max_phys from this map, before exit, so bitmap size is correct. */
    uint64_t max_phys = 0;
    { uint64_t entries = desc_size ? map_size / desc_size : 0;
      for (uint64_t i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* d = (EFI_MEMORY_DESCRIPTOR*)((uint8_t*)mem_map + (i * desc_size));
        if (d->Type == 7) { uint64_t top = d->PhysicalStart + (d->NumberOfPages * 4096); if (top > max_phys) max_phys = top; }
      } }
    uint64_t bitmap_bytes = (max_phys / 4096 + 7) / 8;
    void* bitmap_memory = 0;
    status = bs->AllocatePool(2, bitmap_bytes, &bitmap_memory);
    if (status != EFI_SUCCESS || !bitmap_memory) { serial_write_string("[FAIL] AllocatePool for PMM bitmap failed.\n"); return; }
    /* Exit loop: re-fetch map, then exit. If key is stale, loop again.
       Pasted header omitted ExitBootServices from its BootServices struct,
       so that version would not compile at all. Our uefi.h has it typed. */
    int retries = 0;
    while (retries < 10) {
        status = bs->GetMemoryMap(&map_size, (EFI_MEMORY_DESCRIPTOR*)mem_map, &map_key, &desc_size, &desc_ver);
        if (status != EFI_SUCCESS) { serial_write_string("[FAIL] Memory map re-fetch failed.\n"); return; }
        status = bs->ExitBootServices(image_handle, map_key);
        if (status == EFI_SUCCESS) break;
        retries++;
    }
    if (status != EFI_SUCCESS) { serial_write_string("[FAIL] ExitBootServices failed after 10 retries.\n"); return; }
    serial_write_string("[OK] Exited UEFI Boot Services. CPU is ours.\n");
    pmm_init((EFI_MEMORY_DESCRIPTOR*)mem_map, map_size, desc_size, bitmap_memory, bitmap_bytes, max_phys);
    serial_write_string("[OK] PMM Initialized.\n");
    serial_write_string("[...] Scanning PCIe Bus...\n");
    pcie_scan();
    serial_write_string("[STUB] Halted.\n");
    while (1) { __asm__ volatile("cli; hlt"); }
}
