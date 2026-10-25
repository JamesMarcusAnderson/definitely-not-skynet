#include <stdint.h>
#include "uefi.h"
extern void serial_init(void);
extern void serial_write_string(const char* str);
extern void pcie_scan(void);
static void uefi_print(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* con, const uint16_t* s) { if (con && con->OutputString) con->OutputString(con, s); }
void kmain(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* sys_table) {
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* con = sys_table->ConOut;
    uefi_print(con, (const uint16_t*)u"\nDefinitely Not Skynet: bare-metal Phase 1.1\r\n");
    serial_init();
    serial_write_string("\n=== DEFINITELY NOT SKYNET: BARE METAL ===\n");
    EFI_BOOT_SERVICES* bs = sys_table->BootServices;
    static uint8_t mem_map_buffer[4096 * 8];
    uint64_t map_size = sizeof(mem_map_buffer), map_key = 0, desc_size = 0; uint32_t desc_ver = 0;
    EFI_STATUS status = bs->GetMemoryMap(&map_size, (EFI_MEMORY_DESCRIPTOR*)mem_map_buffer, &map_key, &desc_size, &desc_ver);
    if (status != 0) { uefi_print(con, (const uint16_t*)u"[FAIL] Could not get memory map\r\n"); serial_write_string("[FAIL] Could not get memory map.\n"); return; }
    uefi_print(con, (const uint16_t*)u"[OK] UEFI Memory Map acquired\r\n");
    serial_write_string("[OK] UEFI Memory Map acquired.\n");
    /* Re-fetch immediately before ExitBootServices: any intervening allocation invalidates map_key. */
    map_size = sizeof(mem_map_buffer);
    status = bs->GetMemoryMap(&map_size, (EFI_MEMORY_DESCRIPTOR*)mem_map_buffer, &map_key, &desc_size, &desc_ver);
    if (status != 0) { uefi_print(con, (const uint16_t*)u"[FAIL] Memory map re-fetch failed\r\n"); return; }
    /* FIX vs pasted draft: ExitBootServices takes the IMAGE handle, not ConsoleInHandle. */
    status = bs->ExitBootServices(image_handle, map_key);
    if (status != 0) { serial_write_string("[FAIL] ExitBootServices failed - map_key stale, retry loop needed.\n"); return; }
    /* After this point BootServices / ConOut are gone. Serial only, in QEMU. */
    serial_write_string("[OK] Exited UEFI Boot Services. CPU is ours.\n");
    pcie_scan();
    serial_write_string("[STUB] Reached bare-metal kmain. Halting CPU.\n");
    while (1) { __asm__ volatile("cli; hlt"); }
}
