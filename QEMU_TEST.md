# QEMU Test (not yet run)

Build first on GitHub Actions, then:

```bash
mkdir -p test_efi/EFI/BOOT
cp BOOTX64.EFI test_efi/EFI/BOOT/
qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -drive format=raw,file=fat:rw:test_efi -serial stdio
```

Notes:
- Do NOT use `-nographic` together with `-serial mon:stdio`; they conflict.
  Use `-serial stdio` alone for serial output, or `-nographic` alone.
- OVMF path varies by distro; some use split OVMF_CODE.fd / OVMF_VARS.fd
  with `-drive if=pflash,format=raw,file=...` instead of `-bios`.
- Expected actual output (from this stub, not the pasted example):
  `=== DEFINITELY NOT SKYNET: BARE METAL ===`, `[OK] UEFI Memory Map acquired.`,
  `[OK] Exited UEFI Boot Services. CPU is ours.`, `[OK] PMM Initialized.`,
  then the PCIe scan lines, then `[STUB] Halted.`
- A pass in QEMU without PCI passthrough is a clean boot with no
  Nvidia/AMD GPU found. That is not a failure.
- This test has NOT been run yet. No serial log is claimed here.
- Disk / GGUF loading is NOT in this slice. Next smallest disk step:
  scan for an NVMe controller (Class 0x01, Subclass 0x08) and read
  LBA 0 to check for MBR magic 0x55 0xAA. That proves storage access
  before any filesystem parsing.
