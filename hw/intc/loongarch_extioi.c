/*
 * Loongson 3A5000 ext interrupt controller emulation
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "qemu/module.h"
#include "qemu/log.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "hw/loongarch/loongarch.h"
#include "hw/qdev-properties.h"
#include "exec/address-spaces.h"
#include "hw/intc/loongarch_extioi.h"
#include "migration/vmstate.h"

#define DEBUG_APIC 0

#define DPRINTF(fmt, ...) \
do { \
    if (DEBUG_APIC) { \
        fprintf(stderr, "APIC: " fmt , ## __VA_ARGS__); \
    } \
} while (0)

static void extioi_update_irq(void *opaque, int irq_num, int level)
{
    loongarch_extioi *s = opaque;
    uint8_t  ipnum, cpu;
    unsigned long found1, found2;

    ipnum = s->sw_ipmap[irq_num];
    cpu   = s->sw_coremap[irq_num];
    if (level == 1) {
        if (test_bit(irq_num, (void *)s->en_reg8) == false) {
            return;
        }
        bitmap_set((void *)s->coreisr_reg8[cpu], irq_num, 1);
        found1 = find_next_bit((void *)&(s->sw_ipisr[cpu][ipnum]),
                               EXTIOI_IRQS, 0);
        bitmap_set((void *)&(s->sw_ipisr[cpu][ipnum]), irq_num, 1);

        if (found1 >= EXTIOI_IRQS) {
            qemu_set_irq(s->parent_irq[cpu][ipnum], level);
        }
    } else {
        bitmap_clear((void *)s->coreisr_reg8[cpu], irq_num, 1);
        found1 = find_next_bit((void *)&(s->sw_ipisr[cpu][ipnum]),
                               EXTIOI_IRQS, 0);
        bitmap_clear((void *)&(s->sw_ipisr[cpu][ipnum]), irq_num, 1);
        found2 = find_next_bit((void *)&(s->sw_ipisr[cpu][ipnum]),
                               EXTIOI_IRQS, 0);

        if ((found1 < EXTIOI_IRQS) && (found2 >= EXTIOI_IRQS)) {
            qemu_set_irq(s->parent_irq[cpu][ipnum], level);
        }
    }
}

static void extioi_setirq(void *opaque, int irq, int level)
{
    loongarch_extioi *s = opaque;
    extioi_update_irq(s, irq, level);
}

static void extioi_handler(void *opaque, int irq, int level)
{
    loongarch_extioi *extioi = (loongarch_extioi *)opaque;

    qemu_set_irq(extioi->irq[irq], level);
}

static uint32_t extioi_readb(void *opaque, hwaddr addr)
{
    loongarch_extioi *state = opaque;
    unsigned long offset, reg_count;
    uint8_t ret;
    int cpu;

    offset = addr & 0xffff;

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        reg_count = (offset - EXTIOI_ENABLE_START);
        ret = state->en_reg8[reg_count];
    } else if ((offset >= EXTIOI_BOUNCE_START) &&
               (offset < EXTIOI_BOUNCE_END)) {
        reg_count = (offset - EXTIOI_BOUNCE_START);
        ret = state->bounce_reg8[reg_count];
    } else if ((offset >= EXTIOI_COREISR_START) &&
               (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f);
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;
        ret = state->coreisr_reg8[cpu][reg_count];
    } else if ((offset >= EXTIOI_IPMAP_START) &&
               (offset < EXTIOI_IPMAP_END)) {
        reg_count = (offset - EXTIOI_IPMAP_START);
        ret = state->ipmap_reg8[reg_count];
    } else if ((offset >= EXTIOI_COREMAP_START) &&
               (offset < EXTIOI_COREMAP_END)) {
        reg_count = (offset - EXTIOI_COREMAP_START);
        ret = state->coremap_reg8[reg_count];
    } else if ((offset >= EXTIOI_NODETYPE_START) &&
               (offset < EXTIOI_NODETYPE_END)) {
        reg_count = (offset - EXTIOI_NODETYPE_START);
        ret = state->nodetype_reg8[reg_count];
    }

    DPRINTF("readb reg 0x" TARGET_FMT_plx " = %x\n", addr, ret);
    return ret;
}

static uint32_t extioi_readw(void *opaque, hwaddr addr)
{
    loongarch_extioi *state = opaque;
    unsigned long offset, reg_count;
    uint32_t ret;
    int cpu;

    offset = addr & 0xffff;

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        reg_count = (offset - EXTIOI_ENABLE_START) / 4;
        ret = state->en_reg32[reg_count];
    } else if ((offset >= EXTIOI_BOUNCE_START) &&
               (offset < EXTIOI_BOUNCE_END)) {
        reg_count = (offset - EXTIOI_BOUNCE_START) / 4;
        ret = state->bounce_reg32[reg_count];
    } else if ((offset >= EXTIOI_COREISR_START) &&
               (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f) / 4;
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;
        ret = state->coreisr_reg32[cpu][reg_count];
    } else if ((offset >= EXTIOI_IPMAP_START) &&
               (offset < EXTIOI_IPMAP_END)) {
        reg_count = (offset - EXTIOI_IPMAP_START) / 4;
        ret = state->ipmap_reg32[reg_count];
    } else if ((offset >= EXTIOI_COREMAP_START) &&
               (offset < EXTIOI_COREMAP_END)) {
        reg_count = (offset - EXTIOI_COREMAP_START) / 4;
        ret = state->coremap_reg32[reg_count];
    } else if ((offset >= EXTIOI_NODETYPE_START) &&
               (offset < EXTIOI_NODETYPE_END)) {
        reg_count = (offset - EXTIOI_NODETYPE_START) / 4;
        ret = state->nodetype_reg32[reg_count];
    }

    DPRINTF("readw reg 0x" TARGET_FMT_plx " = %x\n", addr, ret);
    return ret;
}

static uint64_t extioi_readl(void *opaque, hwaddr addr)
{
    loongarch_extioi *state = opaque;
    unsigned long offset, reg_count;
    uint64_t ret;
    int cpu;

    offset = addr & 0xffff;

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        reg_count = (offset - EXTIOI_ENABLE_START) / 8;
        ret = state->en_reg64[reg_count];
    } else if ((offset >= EXTIOI_BOUNCE_START) &&
               (offset < EXTIOI_BOUNCE_END)) {
        reg_count = (offset - EXTIOI_BOUNCE_START) / 8;
        ret = state->bounce_reg64[reg_count];
    } else if ((offset >= EXTIOI_COREISR_START) &&
               (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f) / 8;
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;
        ret = state->coreisr_reg64[cpu][reg_count];
    } else if ((offset >= EXTIOI_IPMAP_START) &&
               (offset < EXTIOI_IPMAP_END)) {
        ret = state->ipmap_reg64;
    } else if ((offset >= EXTIOI_COREMAP_START) &&
               (offset < EXTIOI_COREMAP_END)) {
        reg_count = (offset - EXTIOI_COREMAP_START) / 8;
        ret = state->coremap_reg64[reg_count];
    } else if ((offset >= EXTIOI_NODETYPE_START) &&
               (offset < EXTIOI_NODETYPE_END)) {
        reg_count = (offset - EXTIOI_NODETYPE_START) / 8;
        ret = state->nodetype_reg64[reg_count];
    }

    DPRINTF("readl reg 0x" TARGET_FMT_plx " = %lx\n", addr, ret);
    return ret;
}

static void extioi_writeb(void *opaque, hwaddr addr, uint32_t val)
{
    loongarch_extioi *state = opaque;
    unsigned long offset, reg_count;
    uint8_t old_data_u8;
    int cpu, i, ipnum, level, mask, irqnum;

    offset = addr & 0xffff;
    val = val & 0xffUL;

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        reg_count = (offset - EXTIOI_ENABLE_START);
        old_data_u8 = state->en_reg8[reg_count];
        if (old_data_u8 != val) {
            state->en_reg8[reg_count] = val;
            old_data_u8 = old_data_u8 ^ val;
            mask = 0x1;

            for (i = 0; i < 8; i++) {
                if (old_data_u8 & mask) {
                    level = !!(val & (0x1 << i));
                    extioi_update_irq(state, i + reg_count * 8, level);
                }
                mask = mask << 1;
            }
        }
    } else if ((offset >= EXTIOI_BOUNCE_START) &&
               (offset < EXTIOI_BOUNCE_END)) {
        reg_count = (offset - EXTIOI_BOUNCE_START);
        state->bounce_reg8[reg_count] = val;
    } else if ((offset >= EXTIOI_ISR_START) && (offset < EXTIOI_ISR_END)) {
        /* can not be writen */
        reg_count = (offset - EXTIOI_ISR_START) & 0x1f;
        old_data_u8 = state->isr_reg8[reg_count];
        state->isr_reg8[reg_count] = old_data_u8 & (~val);

        mask = 0x1;
        for (i = 0; i < 8; i++) {
            if ((old_data_u8 & mask) && (val & mask)) {
                extioi_update_irq(state, i + reg_count * 8, 0);
            }
            mask = mask << 1;
        }
    } else if ((offset >= EXTIOI_COREISR_START) &&
               (offset < EXTIOI_COREISR_END)) {
        reg_count = (offset - EXTIOI_COREISR_START) & 0x1f;
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;

        /* ext_isr */
        old_data_u8 = state->isr_reg8[reg_count];
        state->isr_reg8[reg_count] = old_data_u8 & (~val);

        old_data_u8 = state->coreisr_reg8[cpu][reg_count];
        state->coreisr_reg8[cpu][reg_count] = old_data_u8 & (~val);

        if (old_data_u8 != state->coreisr_reg8[cpu][reg_count]) {
            mask = 0x1;
            for (i = 0; i < 8; i++) {
                if ((old_data_u8 & mask) && (val & mask)) {
                    extioi_update_irq(state, i + reg_count * 8, 0);
                }
                mask = mask << 1;
            }
        }
    } else if ((offset >= EXTIOI_IPMAP_START) && (offset < EXTIOI_IPMAP_END)) {
        /* drop arch.core_ip_mask use state->ipmap */
        reg_count = (offset - EXTIOI_IPMAP_START);
        state->ipmap_reg8[reg_count] = val;

        ipnum = 0;
        for (i = 0; i < 4; i++) {
            if (val & (0x1 << i)) {
                ipnum = i;
                break;
            }
        }

        if (val) {
            for (i = 0; i < 32; i++) {
                irqnum = reg_count * 32 + i;
                state->sw_ipmap[irqnum] = ipnum;
            }
        } else {
            for (i = 0; i < 32; i++) {
                irqnum = reg_count * 32 + i;
                state->sw_ipmap[irqnum] = 0;
            }
        }
    } else if ((offset >= EXTIOI_COREMAP_START) &&
               (offset < EXTIOI_COREMAP_END)) {
        reg_count = (offset - EXTIOI_COREMAP_START);
        cpu = val & 0xf;

        /* node map different from kernel */
        if (cpu) {
            cpu = ctz32(cpu);
            state->coremap_reg8[reg_count] = val;
            state->sw_coremap[reg_count] = cpu;
        }
    } else if ((offset >= EXTIOI_NODETYPE_START) &&
               (offset < EXTIOI_NODETYPE_END)) {
        reg_count = (offset - EXTIOI_NODETYPE_START);
        state->nodetype_reg8[reg_count] = val;
    }

    DPRINTF("writeb reg 0x" TARGET_FMT_plx " = %x\n", addr, val);
}

static void extioi_writew(void *opaque, hwaddr addr, uint32_t val)
{
    loongarch_extioi *state = opaque;
    int cpu, level;
    uint32_t offset, old_data_u32, reg_count, mask, i;

    offset = addr & 0xffff;

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        reg_count = (offset - EXTIOI_ENABLE_START) / 4;
        old_data_u32 = state->en_reg32[reg_count];
        if (old_data_u32 != val) {
            state->en_reg32[reg_count] = val;
            old_data_u32 = old_data_u32 ^ val;

            mask = 0x1;
            for (i = 0; i < 8 * sizeof(old_data_u32); i++) {
                if (old_data_u32 & mask) {
                    level = !!(val & (0x1 << i));
                    extioi_update_irq(state, i + reg_count * 32, level);
                }
                mask = mask << 1;
            }
        }
    } else if ((offset >= EXTIOI_BOUNCE_START) &&
               (offset < EXTIOI_BOUNCE_END)) {
        reg_count = (offset - EXTIOI_BOUNCE_START) / 4;
        state->bounce_reg32[reg_count] = val;
    } else if ((offset >= EXTIOI_COREISR_START) &&
               (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f) / 4;
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;

        /* ext_isr */
        old_data_u32 = state->isr_reg32[reg_count];
        state->isr_reg32[reg_count] = old_data_u32 & (~val);

        /* ext_core_ioisr */
        old_data_u32 = state->coreisr_reg32[cpu][reg_count];
        state->coreisr_reg32[cpu][reg_count] = old_data_u32 & (~val);

        if (old_data_u32 != state->coreisr_reg32[cpu][reg_count]) {
            mask = 0x1;
            for (i = 0; i < 8 * sizeof(old_data_u32); i++) {
                if ((old_data_u32 & mask) && (val & mask)) {
                    extioi_update_irq(state, i + reg_count * 8, 0);
                }
                mask = mask << 1;
            }
        }
    } else if ((offset >= EXTIOI_IPMAP_START) &&
               (offset < EXTIOI_IPMAP_END)) {
        extioi_writeb(opaque, addr, (val) & 0xff);
        extioi_writeb(opaque, addr + 1, (val >> 8) & 0xff);
        extioi_writeb(opaque, addr + 2, (val >> 16) & 0xff);
        extioi_writeb(opaque, addr + 3, (val >> 24) & 0xff);
    } else if ((offset >= EXTIOI_COREMAP_START) &&
               (offset < EXTIOI_COREMAP_END)) {
        extioi_writeb(opaque, addr, (val) & 0xff);
        extioi_writeb(opaque, addr + 1, (val >> 8) & 0xff);
        extioi_writeb(opaque, addr + 2, (val >> 16) & 0xff);
        extioi_writeb(opaque, addr + 3, (val >> 24) & 0xff);
    } else if ((offset >= EXTIOI_NODETYPE_START) &&
               (offset < EXTIOI_NODETYPE_END)) {
        reg_count = (offset - EXTIOI_NODETYPE_START) / 4;
        state->nodetype_reg32[reg_count] = val;
    }

    DPRINTF("writew reg 0x" TARGET_FMT_plx " = %x\n", addr, val);
}

static void extioi_writel(void *opaque, hwaddr addr, uint64_t val)
{
    loongarch_extioi *state = (loongarch_extioi *)opaque;
    int cpu, level;
    uint64_t offset, old_data_u64, reg_count, mask, i;

    offset = addr & 0xffff;

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        reg_count = (offset - EXTIOI_ENABLE_START) / 8;
        old_data_u64 = state->en_reg64[reg_count];
        if (old_data_u64 != val) {
            state->en_reg64[reg_count] = val;
            old_data_u64 = old_data_u64 ^ val;
            mask = 0x1;
            for (i = 0; i < 8 * sizeof(old_data_u64); i++) {
                if (old_data_u64 & mask) {
                    level = !!(val & (0x1 << i));
                    extioi_update_irq(state, i + reg_count * 64, level);
                }
                mask = mask << 1;
            }
        }
    } else if ((offset >= EXTIOI_BOUNCE_START) &&
               (offset < EXTIOI_BOUNCE_END)) {
        reg_count = (offset - EXTIOI_BOUNCE_START) / 8;
        state->bounce_reg64[reg_count] = val;
    } else if ((offset >= EXTIOI_COREISR_START) &&
               (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f) / 8;
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;

        /* core_ext_ioisr */
        old_data_u64 = state->coreisr_reg64[cpu][reg_count];
        state->coreisr_reg64[cpu][reg_count] = old_data_u64 & (~val);

        if (old_data_u64 != state->coreisr_reg64[cpu][reg_count]) {
            mask = 0x1;
            for (i = 0; i < 8 * sizeof(old_data_u64); i++) {
                if ((old_data_u64 & mask) && (val & mask)) {
                    extioi_update_irq(state, i + reg_count * 64, 0);
                }
                mask = mask << 1;
            }
        }
    } else if ((offset >= EXTIOI_IPMAP_START) &&
               (offset < EXTIOI_IPMAP_END)) {
        extioi_writeb(opaque, addr, (val) & 0xff);
        extioi_writeb(opaque, addr + 1, (val >> 8) & 0xff);
        extioi_writeb(opaque, addr + 2, (val >> 16) & 0xff);
        extioi_writeb(opaque, addr + 3, (val >> 24) & 0xff);
        extioi_writeb(opaque, addr + 4, (val >> 32) & 0xff);
        extioi_writeb(opaque, addr + 5, (val >> 40) & 0xff);
        extioi_writeb(opaque, addr + 6, (val >> 48) & 0xff);
        extioi_writeb(opaque, addr + 7, (val >> 56) & 0xff);
    } else if ((offset >= EXTIOI_COREMAP_START) &&
               (offset < EXTIOI_COREMAP_END)) {
        extioi_writeb(opaque, addr, (val) & 0xff);
        extioi_writeb(opaque, addr + 1, (val >> 8) & 0xff);
        extioi_writeb(opaque, addr + 2, (val >> 16) & 0xff);
        extioi_writeb(opaque, addr + 3, (val >> 24) & 0xff);
        extioi_writeb(opaque, addr + 4, (val >> 32) & 0xff);
        extioi_writeb(opaque, addr + 5, (val >> 40) & 0xff);
        extioi_writeb(opaque, addr + 6, (val >> 48) & 0xff);
        extioi_writeb(opaque, addr + 7, (val >> 56) & 0xff);
    } else if ((offset >= EXTIOI_NODETYPE_START) &&
               (offset < EXTIOI_NODETYPE_END)) {
        reg_count = (offset - EXTIOI_NODETYPE_START) / 8;
        state->nodetype_reg64[reg_count] = val;
    }

    DPRINTF("writel reg 0x" TARGET_FMT_plx " = %lx\n", addr, val);
}

static uint64_t extioi_readfn(void *opaque, hwaddr addr, unsigned size)
{
    switch (size) {
    case 1:
        return extioi_readb(opaque, addr);
    case 4:
        return extioi_readw(opaque, addr);
    case 8:
        return extioi_readl(opaque, addr);
    default:
        g_assert_not_reached();
    }
}

static void extioi_writefn(void *opaque, hwaddr addr,
                           uint64_t value, unsigned size)
{
    switch (size) {
    case 1:
        extioi_writeb(opaque, addr, value);
        break;
    case 4:
        extioi_writew(opaque, addr, value);
        break;
    case 8:
        extioi_writel(opaque, addr, value);
        break;
    default:
        g_assert_not_reached();
    }
}

static const MemoryRegionOps extioi_ops = {
    .read = extioi_readfn,
    .write = extioi_writefn,
    .impl.min_access_size = 1,
    .impl.max_access_size = 8,
    .valid.min_access_size = 1,
    .valid.max_access_size = 8,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void loongarch_extioi_realize(DeviceState *dev, Error **errp)
{
    LoongarchMachineState *lsms = LOONGARCH_MACHINE(qdev_get_machine());
    MachineState *ms = MACHINE(lsms);
    loongarch_extioi *p = LOONGARCH_EXTIOI(dev);
    int cpu, pin;

    qdev_init_gpio_in(dev, extioi_setirq, EXTIOI_IRQS);

    for (int i = 0; i < EXTIOI_IRQS; i++) {
        sysbus_init_irq(SYS_BUS_DEVICE(dev), &p->irq[i]);
    }

    memory_region_init_io(&p->mmio, OBJECT(p), &extioi_ops, p,
                          TYPE_LOONGARCH_EXTIOI, 0x900);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &p->mmio);

    for (cpu = 0; cpu < ms->smp.cpus; cpu++) {
        for (pin = 0; pin < LS3A_INTC_IP; pin++) {
            sysbus_init_irq(SYS_BUS_DEVICE(dev), &p->parent_irq[cpu][pin]);
        }
    }

    /* 0-31 is for non msi device.32-256 for msi/msix device */
    lsms->pch_irq = qemu_allocate_irqs(extioi_handler, p, 256);
}

static const VMStateDescription vmstate_ext_sw_ipisr = {
    .name = "ext_sw_ipisr",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT8_ARRAY(irq, ext_sw_ipisr, EXTIOI_IRQS_BITMAP_SIZE),
        VMSTATE_END_OF_LIST()
    }
};

static const VMStateDescription vmstate_loongarch_extioi = {
    .name = TYPE_LOONGARCH_EXTIOI,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT8_ARRAY(en_reg8, loongarch_extioi, EXTIOI_IRQS_BITMAP_SIZE),
        VMSTATE_UINT8_ARRAY(bounce_reg8, loongarch_extioi,
                            EXTIOI_IRQS_BITMAP_SIZE),
        VMSTATE_UINT8_ARRAY(isr_reg8, loongarch_extioi,
                            EXTIOI_IRQS_BITMAP_SIZE),
        VMSTATE_UINT8_2DARRAY(coreisr_reg8, loongarch_extioi, MAX_CORES,
                              EXTIOI_IRQS_BITMAP_SIZE),
        VMSTATE_UINT8_ARRAY(ipmap_reg8, loongarch_extioi,
                            EXTIOI_IRQS_IPMAP_SIZE),
        VMSTATE_UINT8_ARRAY(coremap_reg8, loongarch_extioi,
                            EXTIOI_IRQS_COREMAP_SIZE),
        VMSTATE_UINT16_ARRAY(nodetype_reg16, loongarch_extioi,
                             EXTIOI_IRQS_NODETYPE_SIZE),
        VMSTATE_UINT8_ARRAY(sw_ipmap, loongarch_extioi, EXTIOI_IRQS),
        VMSTATE_UINT8_ARRAY(sw_coremap, loongarch_extioi, EXTIOI_IRQS),
        VMSTATE_STRUCT_2DARRAY(sw_ipisr, loongarch_extioi, MAX_CORES,
                               LS3A_INTC_IP, 1, vmstate_ext_sw_ipisr,
                               ext_sw_ipisr),
        VMSTATE_END_OF_LIST()
    }
};

static void loongarch_extioi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->vmsd = &vmstate_loongarch_extioi;
    dc->realize = loongarch_extioi_realize;
}

static const TypeInfo loongarch_extioi_info = {
    .name          = TYPE_LOONGARCH_EXTIOI,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(struct loongarch_extioi),
    .class_init    = loongarch_extioi_class_init,
};

static void loongarch_extioi_register_types(void)
{
    type_register_static(&loongarch_extioi_info);
}

type_init(loongarch_extioi_register_types)
