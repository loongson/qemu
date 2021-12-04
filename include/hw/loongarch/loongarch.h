/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Definitions for loongarch board emulation.
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 */

#ifndef HW_LOONGARCH_H
#define HW_LOONGARCH_H

#include "target/loongarch/cpu.h"
#include "qemu-common.h"
#include "hw/boards.h"
#include "qemu/queue.h"

#define LOONGARCH_MAX_VCPUS     4
#define PM_MMIO_ADDR            0x10080000UL
#define PM_MMIO_SIZE            0x100
#define PM_CNT_MODE             0x10
#define FEATURE_REG             0x8
#define IOCSRF_TEMP             0
#define IOCSRF_NODECNT          1
#define IOCSRF_MSI              2
#define IOCSRF_EXTIOI           3
#define IOCSRF_CSRIPI           4
#define IOCSRF_FREQCSR          5
#define IOCSRF_FREQSCALE        6
#define IOCSRF_DVFSV1           7
#define IOCSRF_GMOD             9
#define IOCSRF_VM               11

#define VENDOR_REG              0x10
#define CPUNAME_REG             0x20

#define FW_CFG_ADDR             0x1e020000
#define LA_BIOS_BASE            0x1c000000
#define LA_BIOS_SIZE            (4 * 1024 * 1024)

/* Kernels can be configured with 64KB pages */
#define INITRD_PAGE_SIZE        (64 * KiB)
#define INITRD_BASE             0x04000000
#define COMMAND_LINE_SIZE       4096

typedef struct LoongArchMachineState {
    /*< private >*/
    MachineState parent_obj;

    AddressSpace address_space_iocsr;
    MemoryRegion system_iocsr;
    MemoryRegion lowmem;
    MemoryRegion highmem;
    MemoryRegion bios;

    /* State for other subsystems/APIs: */
    FWCfgState  *fw_cfg;
} LoongArchMachineState;

#define TYPE_LOONGARCH_MACHINE  MACHINE_TYPE_NAME("loongson3-ls7a")
DECLARE_INSTANCE_CHECKER(LoongArchMachineState, LOONGARCH_MACHINE,
                         TYPE_LOONGARCH_MACHINE)
#endif
