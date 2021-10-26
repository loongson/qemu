/*
 * QEMU Loongson 7A1000 msi interrupt controller.
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "hw/irq.h"
#include "hw/intc/loongarch_pch_msi.h"
#include "hw/pci/msi.h"
#include "hw/misc/unimp.h"
#include "migration/vmstate.h"

#define DEBUG_LOONGARCH_PCH_MSI 0

#define DPRINTF(fmt, ...) \
do { \
    if (DEBUG_LOONGARCH_PCH_MSI) { \
        fprintf(stderr, "LOONGARCH_PCH_MSI: " fmt , ## __VA_ARGS__); \
    } \
} while (0)

static uint64_t loongarch_msi_mem_read(void *opaque, hwaddr addr, unsigned size)
{
    return 0;
}

static void loongarch_msi_mem_write(void *opaque, hwaddr addr,
                                    uint64_t val, unsigned size)
{
    loongarch_pch_msi *s = opaque;
    int irq_num = val & 0xff;

    qemu_set_irq(s->pch_msi_irq[irq_num - 32], 1);
}

static const MemoryRegionOps loongarch_pch_msi_ops = {
    .read  = loongarch_msi_mem_read,
    .write = loongarch_msi_mem_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void loongarch_pch_msi_init(Object *obj)
{
    loongarch_pch_msi *s = LOONGARCH_PCH_MSI(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    int tmp;

    memory_region_init_io(&s->msi_mmio, obj, &loongarch_pch_msi_ops,
                          s, TYPE_LOONGARCH_PCH_MSI, 0x8);
    sysbus_init_mmio(sbd, &s->msi_mmio);
    msi_nonbroken = true;

    for (tmp = 0; tmp < 224; tmp++) {
        sysbus_init_irq(sbd, &s->pch_msi_irq[tmp]);
    }
}

static const TypeInfo loongarch_pch_msi_info = {
    .name          = TYPE_LOONGARCH_PCH_MSI,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(loongarch_pch_msi),
    .instance_init = loongarch_pch_msi_init,
};

static void loongarch_pch_msi_register_types(void)
{
    type_register_static(&loongarch_pch_msi_info);
}

type_init(loongarch_pch_msi_register_types)
