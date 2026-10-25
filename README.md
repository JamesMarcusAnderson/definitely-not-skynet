# Definitely Not Skynet

This is a secret project name for a bare-metal rebuild in progress. It is not a singularity engine yet. Phase 1 is CPU bare-metal inference. Phase 2 is a bare-metal GPU driver and dispatch layer. No benchmarks are claimed until they are measured on real hardware.

## Phase 1.1

Phase 1.1 stub files are added. They are syntax-checked only. They are NOT yet boot-tested. GitHub Actions build validation is the next check. No benchmark claims are made.

## Phase 0

A PCIe scan is added after ExitBootServices. It finds Nvidia 0x10DE or AMD 0x1002 devices and prints BAR0. Finding a BAR is NOT a GPU driver. No GPU commands are sent, and no inference is run. This is NOT yet boot-tested. GitHub Actions build validation is the next check. No benchmark claims are made.
## Phase 1.2

PMM added. The bitmap size is calculated from max physical memory, not fixed 1MB. Bitmap pages are reserved. ExitBootServices uses a retry loop. This is NOT yet boot-tested. GitHub Actions build validation is the next check. No benchmark claims are made. Disk/GGUF loading is not in this slice.
