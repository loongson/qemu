/*
 * Loongarch ipi interrupt support
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qapi/error.h"
#include "exec/address-spaces.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "sysemu/sysemu.h"
#include "sysemu/cpus.h"
#include "cpu.h"
#include "qemu/log.h"
#include "hw/loongarch/loongarch.h"
#include "migration/vmstate.h"

static const VMStateDescription vmstate_gipi_core = {
    .name = "gipi-single",
    .version_id = 0,
    .minimum_version_id = 0,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32(status, gipi_core),
        VMSTATE_UINT32(en, gipi_core),
        VMSTATE_UINT32(set, gipi_core),
        VMSTATE_UINT32(clear, gipi_core),
        VMSTATE_UINT64_ARRAY(buf, gipi_core, MAX_GIPI_MBX_NUM),
        VMSTATE_END_OF_LIST()
    }
};

static const VMStateDescription vmstate_gipi = {
    .name = "gipi",
    .version_id = 0,
    .minimum_version_id = 0,
    .fields = (VMStateField[]) {
        VMSTATE_STRUCT_ARRAY(core, gipiState, MAX_GIPI_CORE_NUM, 0,
                             vmstate_gipi_core, gipi_core),
        VMSTATE_END_OF_LIST()
    }
};

static void gipi_writel(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    gipi_core *s = opaque;
    void *pbuf;

    if (size != 4) {
        hw_error("size not 4");
    }
    addr &= 0xff;
    switch (addr) {
    case CORE_STATUS_OFF:
        hw_error("CORE_SET_OFF Can't be write\n");
        break;
    case CORE_EN_OFF:
        s->en = val;
        break;
    case CORE_SET_OFF:
        s->status |= val;
        if (s->status != 0) {
            qemu_irq_raise(s->irq);
        }
        break;
    case CORE_CLEAR_OFF:
        s->status ^= val;
        if (s->status == 0) {
            qemu_irq_lower(s->irq);
        }
        break;
    case CORE_BUF_20 ... CORE_BUF_38:
        pbuf =  (void *)s->buf + (addr - 0x20);
        *(unsigned int *)pbuf = val;
        break;
    default:
        break;
    }
}

static uint64_t gipi_readl(void *opaque, hwaddr addr, unsigned size)
{
    gipi_core *s = opaque;
    uint64_t ret = 0;
    void *pbuf;

    addr &= 0xff;
    if (size != 4) {
        hw_error("size not 4 %d\n", size);
    }
    switch (addr) {
    case CORE_STATUS_OFF:
        ret = s->status;
        break;
    case CORE_EN_OFF:
        ret = s->en;
        break;
    case CORE_SET_OFF:
        ret = 0;
        break;
    case CORE_CLEAR_OFF:
        ret = 0;
        break;
    case CORE_BUF_20 ... CORE_BUF_38:
        pbuf =  (void *)s->buf + (addr - 0x20);
        ret = *(unsigned int *)pbuf;
        break;
    default:
        break;
    }

    return ret;
}

static const MemoryRegionOps gipi_ops = {
    .read = gipi_readl,
    .write = gipi_writel,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

int cpu_init_ipi(LoongarchMachineState *lsms, qemu_irq parent, int cpu)
{
    int core_num = cpu % 4;
    hwaddr addr;
    MemoryRegion *region;
    char str[32];

    if (lsms->gipi == NULL) {
        lsms->gipi = g_malloc0(sizeof(gipiState));
        vmstate_register(NULL, 0, &vmstate_gipi, lsms->gipi);
    }

    lsms->gipi->core[cpu].irq = parent;

    addr = SMP_GIPI_MAILBOX + core_num * 0x100;
    region = g_new(MemoryRegion, 1);
    sprintf(str, "gipi%d", cpu);
    memory_region_init_io(region, NULL, &gipi_ops,
                          &lsms->gipi->core[cpu], str, 0x100);
    memory_region_add_subregion(get_system_memory(), addr, region);
    return 0;
}
