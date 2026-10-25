#include <stdint.h>
#include "uefi.h"
extern void kmain(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* sys_table);
EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* sys_table) {
    kmain(image_handle, sys_table);
    return 0;
}
