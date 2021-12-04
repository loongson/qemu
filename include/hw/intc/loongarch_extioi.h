/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * LoongArch 3A5000 ext interrupt controller definitions
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 */

#include "hw/sysbus.h"
#include "hw/loongarch/loongarch.h"

#ifndef LOONGARCH_EXTIOI_H
#define LOONGARCH_EXTIOI_H

#define LS3A_INTC_IP               8
#define MAX_CORES                  LOONGARCH_MAX_VCPUS
#define EXTIOI_IRQS                (256)
#define EXTIOI_IRQS_BITMAP_SIZE    (256 / 8)
/* map to ipnum per 32 irqs */
#define EXTIOI_IRQS_IPMAP_SIZE     (256 / 32)
#define EXTIOI_IRQS_COREMAP_SIZE   256
#define EXTIOI_IRQS_NODETYPE_SIZE  16

#define APIC_OFFSET                  0x400
#define APIC_BASE                    (0x1000ULL + APIC_OFFSET)

#define EXTIOI_NODETYPE_START        (0x4a0 - APIC_OFFSET)
#define EXTIOI_NODETYPE_END          (0x4c0 - APIC_OFFSET)
#define EXTIOI_IPMAP_START           (0x4c0 - APIC_OFFSET)
#define EXTIOI_IPMAP_END             (0x4c8 - APIC_OFFSET)
#define EXTIOI_ENABLE_START          (0x600 - APIC_OFFSET)
#define EXTIOI_ENABLE_END            (0x620 - APIC_OFFSET)
#define EXTIOI_BOUNCE_START          (0x680 - APIC_OFFSET)
#define EXTIOI_BOUNCE_END            (0x6a0 - APIC_OFFSET)
#define EXTIOI_ISR_START             (0x700 - APIC_OFFSET)
#define EXTIOI_ISR_END               (0x720 - APIC_OFFSET)
#define EXTIOI_COREISR_START         (0x800 - APIC_OFFSET)
#define EXTIOI_COREISR_END           (0xB20 - APIC_OFFSET)
#define EXTIOI_COREMAP_START         (0xC00 - APIC_OFFSET)
#define EXTIOI_COREMAP_END           (0xD00 - APIC_OFFSET)

#define TYPE_LOONGARCH_EXTIOI "loongarch.extioi"
DECLARE_INSTANCE_CHECKER(struct loongarch_extioi, LOONGARCH_EXTIOI,
                         TYPE_LOONGARCH_EXTIOI)

typedef struct ext_sw_ipisr {
    uint8_t irq[EXTIOI_IRQS_BITMAP_SIZE];
} ext_sw_ipisr;

typedef struct loongarch_extioi {
    SysBusDevice parent_obj;
    /* hardware state */
    uint64_t enable[EXTIOI_IRQS_BITMAP_SIZE / 8];
    uint64_t bounce[EXTIOI_IRQS_BITMAP_SIZE / 8];
    uint64_t coreisr[MAX_CORES][EXTIOI_IRQS_BITMAP_SIZE / 8];
    uint64_t ipmap;
    uint64_t coremap[EXTIOI_IRQS_COREMAP_SIZE / 8];
    uint64_t nodetype[EXTIOI_IRQS_NODETYPE_SIZE / 4];

    /*software state */
    uint8_t sw_ipmap[EXTIOI_IRQS];
    uint8_t sw_coremap[EXTIOI_IRQS];
    ext_sw_ipisr sw_ipisr[MAX_CORES][LS3A_INTC_IP];

    qemu_irq parent_irq[MAX_CORES][LS3A_INTC_IP];
    qemu_irq irq[EXTIOI_IRQS];
    MemoryRegion mmio;
} loongarch_extioi;

#endif /* LOONGARCH_EXTIOI_H */
