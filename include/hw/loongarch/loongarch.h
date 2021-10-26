/*
 * Definitions for loongarch board emulation.
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef HW_LOONGARCH_H
#define HW_LOONGARCH_H

#include "target/loongarch/cpu.h"
#include "qemu-common.h"
#include "hw/boards.h"
#include "qemu/queue.h"
#include "hw/loongarch/gipi.h"

#define LOONGARCH_MAX_VCPUS     4
#define PM_MMIO_ADDR            0x10080000UL
#define PM_MMIO_SIZE            0x100
#define PM_CNT_MODE             0x10
#define FEATURE_REG             0x1fe00008
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

#define VENDOR_REG              0x1fe00010
#define CPUNAME_REG             0x1fe00020
#define MISC_FUNC_REG           0x1fe00420
#define FREQ_REG                0x1fe001d0
#define FW_CFG_ADDR             0x1e020000

#define LA_BIOS_BASE            0x1c000000
#define LA_BIOS_SIZE            (4 * 1024 * 1024)

/* Kernels can be configured with 64KB pages */
#define INITRD_PAGE_SIZE        (64 * KiB)
#define INITRD_BASE             0x04000000
#define COMMAND_LINE_SIZE       4096
#define PHYS_TO_VIRT(x)         ((x) | 0x9000000000000000ULL)

typedef struct LoongarchMachineState {
    /*< private >*/
    MachineState parent_obj;

    /* State for other subsystems/APIs: */
    Notifier machine_done;
    gipiState   *gipi;
    qemu_irq    *pch_irq;
    FWCfgState  *fw_cfg;
} LoongarchMachineState;

#define TYPE_LOONGARCH_MACHINE  MACHINE_TYPE_NAME("loongson7a")
DECLARE_INSTANCE_CHECKER(LoongarchMachineState, LOONGARCH_MACHINE,
                         TYPE_LOONGARCH_MACHINE)

void cpu_loongarch_init_irq(LoongArchCPU *cpu);
int cpu_init_ipi(LoongarchMachineState *ms, qemu_irq parent, int cpu);
#endif
