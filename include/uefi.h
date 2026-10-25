#pragma once
#include <stdint.h>
#include <stddef.h>
#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0
#define EFI_BUFFER_TOO_SMALL 5
typedef uint64_t EFI_STATUS;
typedef void* EFI_HANDLE;
typedef uint64_t EFI_PHYSICAL_ADDRESS;
typedef uint64_t EFI_VIRTUAL_ADDRESS;
typedef struct { uint32_t Data1; uint16_t Data2; uint16_t Data3; uint8_t Data4[8]; } EFI_GUID;
typedef struct { uint32_t Type; uint32_t Pad; EFI_PHYSICAL_ADDRESS PhysicalStart; EFI_VIRTUAL_ADDRESS VirtualStart; uint64_t NumberOfPages; uint64_t Attribute; } EFI_MEMORY_DESCRIPTOR;
typedef struct { uint64_t Signature; uint32_t Revision; uint32_t HeaderSize; uint32_t CRC32; uint32_t Reserved; } EFI_TABLE_HEADER;
/* Simple Text Output - the UEFI console, works in QEMU/OVMF. COM1 does not exist on iMac19,1. */
typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    void* Reset;
    EFI_STATUS (EFIAPI *OutputString)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This, const uint16_t* String);
    void* QueryMode; void* SetMode; void* SetAttribute; void* ClearScreen; void* SetCursorPosition; void* EnableCursor; void* Mode;
};
typedef struct {
    EFI_TABLE_HEADER Hdr; void* GetTime; void* SetTime; void* GetWakeupTime; void* SetWakeupTime; void* SetVirtualAddressMap; void* ConvertPointer; void* GetVariable; void* GetNextVariableName; void* SetVariable; void* GetNextHighMonotonicCount; void* ResetSystem;
} EFI_RUNTIME_SERVICES;
typedef struct {
    EFI_TABLE_HEADER Hdr; void* RaiseTPL; void* RestoreTPL; void* AllocatePages; void* FreePages;
    EFI_STATUS (EFIAPI *GetMemoryMap)(uint64_t* MemoryMapSize, EFI_MEMORY_DESCRIPTOR* MemoryMap, uint64_t* MapKey, uint64_t* DescriptorSize, uint32_t* DescriptorVersion);
    EFI_STATUS (EFIAPI *AllocatePool)(uint32_t PoolType, uint64_t Size, void** Buffer); void* FreePool; void* CreateEvent; void* SetTimer; void* WaitForEvent; void* SignalEvent; void* CloseEvent; void* CheckEvent; void* InstallProtocolInterface; void* ReinstallProtocolInterface; void* UninstallProtocolInterface; void* HandleProtocol; void* Reserved; void* RegisterProtocolNotify; void* LocateHandle; void* LocateDevicePath; void* InstallConfigurationTable; void* LoadImage; void* StartImage; void* Exit; void* UnloadImage;
    EFI_STATUS (EFIAPI *ExitBootServices)(EFI_HANDLE ImageHandle, uint64_t MapKey);
} EFI_BOOT_SERVICES;
typedef struct {
    EFI_TABLE_HEADER Hdr; uint16_t* FirmwareVendor; uint32_t FirmwareRevision; EFI_HANDLE ConsoleInHandle; void* ConIn; EFI_HANDLE ConsoleOutHandle; EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut; EFI_HANDLE StandardErrorHandle; void* StdErr; EFI_RUNTIME_SERVICES* RuntimeServices; EFI_BOOT_SERVICES* BootServices; uint64_t NumberOfTableEntries; void* ConfigurationTable;
} EFI_SYSTEM_TABLE;
