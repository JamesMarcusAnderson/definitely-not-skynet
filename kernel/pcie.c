#include <stdint.h>
#include "uefi.h"
extern void serial_write_string(const char* str);
extern void serial_write_char(char c);

/* QEMU-friendly output: serial is the reliable log channel.
   VGA text at 0xB8000 is kept as a secondary display for QEMU text mode,
   but it is NOT a reliable display on modern UEFI/GOP systems or on Apple
   hardware, which do not guarantee a legacy VGA text buffer. */
static inline void outl(uint16_t port, uint32_t val) { __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port)); }
static inline uint32_t inl(uint16_t port) { uint32_t ret; __asm__ volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port)); return ret; }

static int vga_row = 0, vga_col = 0;
static void vga_put(char c) {
    if (c == '\n') { vga_col = 0; vga_row++; if (vga_row >= 25) vga_row = 0; return; }
    if (vga_col >= 80) { vga_col = 0; vga_row++; if (vga_row >= 25) vga_row = 0; }
    char* video = (char*)0xB8000;
    int off = (vga_row * 80 + vga_col) * 2;
    video[off] = c; video[off + 1] = 0x0F; vga_col++;
}
static void out_both(const char* str) {
    serial_write_string(str);
    while (*str) vga_put(*str++);
}
static void out_hex(uint64_t val) {
    char buf[19]; buf[0]='0'; buf[1]='x';
    for (int i = 0; i < 16; i++) {
        uint8_t n = (val >> ((15 - i) * 4)) & 0xF;
        buf[2 + i] = n < 10 ? '0' + n : 'A' + (n - 10);
    }
    buf[18] = 0;
    serial_write_string(buf);
    const char* s = buf; while (*s) vga_put(*s++);
}
static void out_u16(uint32_t v) { /* decimal-free: reuse hex, caller labels it */ out_hex(v); }

void pcie_scan(void) {
    out_both("Scanning PCIe bus for GPUs (legacy config ports 0xCF8/0xCFC)...\n");
    out_both("Note: finding a BAR is not a GPU driver. No commands are sent yet.\n");
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t addr = (1u << 31) | ((uint32_t)bus << 16) | ((uint32_t)dev << 11) | ((uint32_t)func << 8);
                outl(0xCF8, addr);
                uint32_t dword0 = inl(0xCFC);
                uint32_t vendor = dword0 & 0xFFFF;
                if (vendor == 0xFFFF) continue;
                uint32_t device = (dword0 >> 16) & 0xFFFF;
                if (vendor == 0x10DE || vendor == 0x1002) {
                    out_both("[GPU FOUND] Vendor: "); out_hex(vendor);
                    out_both(" Device: "); out_hex(device);
                    out_both(" Bus: "); out_u16(bus);
                    out_both(" Dev: "); out_u16(dev);
                    out_both(" Func: "); out_u16(func); out_both("\n");
                    outl(0xCF8, addr | 0x10);
                    uint32_t bar0 = inl(0xCFC);
                    if (bar0 & 0x1) {
                        out_both("[GPU BAR0] I/O BAR, not memory-mapped. Skipping.\n");
                    } else if (((bar0 >> 1) & 0x3) == 0x2) {
                        /* 64-bit memory BAR: bit 3 is prefetchable, it must NOT
                           be used to reject the BAR. Pasted draft checked
                           (bar0 & 0x0F) == 0x04, which rejects prefetchable BARs. */
                        outl(0xCF8, addr | 0x14);
                        uint32_t bar0_high = inl(0xCFC);
                        uint64_t full = ((uint64_t)bar0_high << 32) | (bar0 & 0xFFFFFFF0u);
                        out_both("[GPU MMIO BAR0] 64-bit, mapped at: "); out_hex(full); out_both("\n");
                    } else {
                        out_both("[GPU MMIO BAR0] 32-bit, mapped at: "); out_hex(bar0 & 0xFFFFFFF0u); out_both("\n");
                    }
                }
            }
        }
    }
    out_both("PCIe scan complete. Halting CPU.\n");
}
