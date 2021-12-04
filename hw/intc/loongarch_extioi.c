/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Loongson 3A5000 ext interrupt controller emulation
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
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
#include "trace.h"

static void extioi_update_irq(void *opaque, int irq_num, int level)
{
    loongarch_extioi *s = LOONGARCH_EXTIOI(opaque);
    uint8_t  ipnum, cpu;
    unsigned long found1, found2;

    ipnum = s->sw_ipmap[irq_num];
    cpu   = s->sw_coremap[irq_num];
    if (level == 1) {
        if (test_bit(irq_num, (void *)s->enable) == false) {
            return;
        }
        bitmap_set((void *)s->coreisr[cpu], irq_num, 1);
        found1 = find_next_bit((void *)&(s->sw_ipisr[cpu][ipnum]),
                               EXTIOI_IRQS, 0);
        bitmap_set((void *)&(s->sw_ipisr[cpu][ipnum]), irq_num, 1);

        if (found1 >= EXTIOI_IRQS) {
            qemu_set_irq(s->parent_irq[cpu][ipnum], level);
        }
    } else {
        bitmap_clear((void *)s->coreisr[cpu], irq_num, 1);
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
    loongarch_extioi *s = LOONGARCH_EXTIOI(opaque);
    trace_extioi_setirq(irq, level);
    extioi_update_irq(s, irq, level);
}

static uint32_t extioi_readb(void *opaque, hwaddr addr)
{
    loongarch_extioi *s = LOONGARCH_EXTIOI(opaque);
    unsigned long offset, reg_count;
    uint8_t ret;
    int cpu;

    offset = addr & 0xffff;

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        ret = ldub_p((void *)s->enable + (offset - EXTIOI_ENABLE_START));
    } else if ((offset >= EXTIOI_BOUNCE_START) && (offset < EXTIOI_BOUNCE_END)) {
        ret = ldub_p((void *)s->bounce + (offset - EXTIOI_BOUNCE_START));
    } else if ((offset >= EXTIOI_COREISR_START) && (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f);
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;
        ret = ldub_p((void *)s->coreisr[cpu] + reg_count);
    } else if ((offset >= EXTIOI_IPMAP_START) && (offset < EXTIOI_IPMAP_END)) {
        ret = ldub_p((void *)&s->ipmap + (offset - EXTIOI_IPMAP_START));
    } else if ((offset >= EXTIOI_COREMAP_START) && (offset < EXTIOI_COREMAP_END)) {
        ret = ldub_p((void *)s->coremap + (offset - EXTIOI_COREMAP_START));
    } else if ((offset >= EXTIOI_NODETYPE_START) && (offset < EXTIOI_NODETYPE_END)) {
        ret = ldub_p((void *)s->nodetype + (offset - EXTIOI_NODETYPE_START));
    }

    trace_loongarch_extioi_readb((uint32_t)addr, ret);
    return ret;
}

static uint32_t extioi_readw(void *opaque, hwaddr addr)
{
    loongarch_extioi *s = LOONGARCH_EXTIOI(opaque);
    unsigned long offset, reg_count;
    uint32_t ret;
    int cpu;

    offset = addr & 0xffff;

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        ret = ldl_p((void *)s->enable + (offset - EXTIOI_ENABLE_START));
    } else if ((offset >= EXTIOI_BOUNCE_START) && (offset < EXTIOI_BOUNCE_END)) {
        ret = ldl_p((void *)s->bounce + (offset - EXTIOI_BOUNCE_START));
    } else if ((offset >= EXTIOI_COREISR_START) && (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f);
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;
        ret = ldl_p((void *)s->coreisr[cpu] + reg_count);
    } else if ((offset >= EXTIOI_IPMAP_START) && (offset < EXTIOI_IPMAP_END)) {
        ret = ldl_p((void *)&s->ipmap + (offset - EXTIOI_IPMAP_START));
    } else if ((offset >= EXTIOI_COREMAP_START) && (offset < EXTIOI_COREMAP_END)) {
        ret = ldl_p((void *)s->coremap + (offset - EXTIOI_COREMAP_START));
    } else if ((offset >= EXTIOI_NODETYPE_START) && (offset < EXTIOI_NODETYPE_END)) {
        ret = ldl_p((void *)s->nodetype + (offset - EXTIOI_NODETYPE_START));
    }

    trace_loongarch_extioi_readw((uint32_t)addr, ret);
    return ret;
}

static uint64_t extioi_readl(void *opaque, hwaddr addr)
{
    loongarch_extioi *s = LOONGARCH_EXTIOI(opaque);
    unsigned long offset, reg_count;
    uint64_t ret;
    int cpu;

    offset = addr & 0xffff;

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        ret = ldq_p((void *)s->enable + (offset - EXTIOI_ENABLE_START));
    } else if ((offset >= EXTIOI_BOUNCE_START) && (offset < EXTIOI_BOUNCE_END)) {
        ret = ldq_p((void *)s->bounce + (offset - EXTIOI_BOUNCE_START));
    } else if ((offset >= EXTIOI_COREISR_START) && (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f);
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;
        ret = ldq_p((void *)s->coreisr[cpu] + reg_count);
    } else if ((offset >= EXTIOI_IPMAP_START) && (offset < EXTIOI_IPMAP_END)) {
        ret = ldq_p((void *)&s->ipmap + (offset - EXTIOI_IPMAP_START));
    } else if ((offset >= EXTIOI_COREMAP_START) && (offset < EXTIOI_COREMAP_END)) {
        ret = ldq_p((void *)s->coremap + (offset - EXTIOI_COREMAP_START));
    } else if ((offset >= EXTIOI_NODETYPE_START) && (offset < EXTIOI_NODETYPE_END)) {
        ret = ldq_p((void *)s->nodetype + (offset - EXTIOI_NODETYPE_START));
    }

    trace_loongarch_extioi_readl((uint32_t)addr, ret);
    return ret;
}

static void extioi_writeb(void *opaque, hwaddr addr, uint32_t val,
                          unsigned size)
{
    loongarch_extioi *s = LOONGARCH_EXTIOI(opaque);
    unsigned long offset, reg_count;
    uint8_t old_data;
    int cpu, i, j, ipnum, level, irqnum, bits;

    offset = addr & 0xffff;
    val = val & 0xffUL;

    trace_loongarch_extioi_writeb(size, (uint32_t)addr, val);
    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        reg_count = (offset - EXTIOI_ENABLE_START);
        old_data = ldub_p((void *)s->enable + reg_count);
        if (old_data != val) {
            stb_p((void *)s->enable + reg_count, val);
            old_data = old_data ^ val;
            bits = size * 8;
            while ((i = find_first_bit((void *)&old_data, bits)) != bits) {
                level = test_bit(i, (unsigned long *)&val);
                extioi_update_irq(s, i + reg_count * 8, level);
                clear_bit(i, (void *)&old_data);
            }
        }
    } else if ((offset >= EXTIOI_BOUNCE_START) && (offset < EXTIOI_BOUNCE_END)) {
        stb_p((void *)s->bounce + (offset - EXTIOI_BOUNCE_START), val);
    } else if ((offset >= EXTIOI_COREISR_START) && (offset < EXTIOI_COREISR_END)) {
        reg_count = (offset - EXTIOI_COREISR_START) & 0x1f;
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;

        old_data = ldub_p((void *)s->coreisr[cpu] + reg_count);
        stb_p((void *)s->coreisr[cpu] + reg_count, (old_data & ~val));

        if (old_data != (old_data & ~val)) {
            bits = size * 8;

            while ((i = find_first_bit((void *)&val, bits)) != bits) {
                j = test_bit(i, (unsigned long *)&old_data);
                if (j) {
                    extioi_update_irq(s, i + reg_count * 8, 0);
                }
                clear_bit(i, (void *)&val);
            }
        }
    } else if ((offset >= EXTIOI_IPMAP_START) && (offset < EXTIOI_IPMAP_END)) {
        /* Drop arch.core_ip_mask use s->ipmap */
        reg_count = (offset - EXTIOI_IPMAP_START);
        stb_p((void *)&s->ipmap + reg_count, val);

        /* Routing in groups of 32 interrupt */
        ipnum = find_first_bit((void *)&val, 4);
        for (i = 0; i < 32; i++) {
            irqnum = reg_count * 32 + i;
            if (ipnum != 4) {
                s->sw_ipmap[irqnum] = ipnum;
            } else {
                s->sw_ipmap[irqnum] = 0;
            }
        }
    } else if ((offset >= EXTIOI_COREMAP_START) && (offset < EXTIOI_COREMAP_END)) {
        reg_count = (offset - EXTIOI_COREMAP_START);
        stb_p((void *)s->coremap + reg_count, val);

        /* Only map the core */
        if (val) {
            cpu = find_first_bit((void *)&val, 4);
            if (cpu != 4) {
                s->sw_coremap[reg_count] = cpu;
            }
        }
    } else if ((offset >= EXTIOI_NODETYPE_START) && (offset < EXTIOI_NODETYPE_END)) {
        stb_p((void *)s->nodetype + (offset - EXTIOI_NODETYPE_START), val);
    }
}

static void extioi_writew(void *opaque, hwaddr addr, uint32_t val,
                          unsigned size)
{
    loongarch_extioi *s = LOONGARCH_EXTIOI(opaque);
    int cpu, level, irqnum, ipnum;
    uint32_t offset, old_data, reg_count, i, j, bits;

    offset = addr & 0xffff;
    trace_loongarch_extioi_writew(size, (uint32_t)addr, val);

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        reg_count = (offset - EXTIOI_ENABLE_START);
        old_data = ldl_p((void *)s->enable + reg_count);
        if (old_data != val) {
            stl_p((void *)s->enable + reg_count, val);
            old_data = old_data ^ val;

            bits = size * 8;
            while ((i = find_first_bit((void *)&old_data, bits)) != bits) {
                level = test_bit(i, (unsigned long *)&val);
                extioi_update_irq(s, i + reg_count * 8, level);
                clear_bit(i, (void *)&old_data);
            }
        }
    } else if ((offset >= EXTIOI_BOUNCE_START) && (offset < EXTIOI_BOUNCE_END)) {
        stl_p((void *)s->bounce + (offset - EXTIOI_BOUNCE_START), val);
    } else if ((offset >= EXTIOI_COREISR_START) && (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f);
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;

        /* Ext_core_ioisr */
        old_data = ldl_p((void *)s->coreisr[cpu] + reg_count);
        stl_p((void *)s->coreisr[cpu] + reg_count, (old_data & ~val));

        if (old_data != (old_data & ~val)) {
            bits = size * 8;
            while ((i = find_first_bit((void *)&val, bits)) != bits) {
                j = test_bit(i, (unsigned long *)&old_data);
                if (j) {
                    extioi_update_irq(s, i + reg_count * 8, 0);
                }
                clear_bit(i, (void *)&val);
            }
        }
    } else if ((offset >= EXTIOI_IPMAP_START) && (offset < EXTIOI_IPMAP_END)) {
        /* Drop arch.core_ip_mask use s->ipmap */
        reg_count = (offset - EXTIOI_IPMAP_START);
        stl_p((void *)&s->ipmap + reg_count, val);

        /* Routing in groups of 32 interrupt */
        while (val) {
            ipnum = find_first_bit((void *)&val, 4);
            for (i = 0; i < 32; i++) {
                irqnum = reg_count * 32 + i;
                if (ipnum != 4) {
                    s->sw_ipmap[irqnum] = ipnum;
                } else {
                    s->sw_ipmap[irqnum] = 0;
                }
            }
            val = val >> 8;
            reg_count += 1;
        }
    } else if ((offset >= EXTIOI_COREMAP_START) && (offset < EXTIOI_COREMAP_END)) {
        reg_count = (offset - EXTIOI_COREMAP_START) / 4;
        stl_p((void *)s->coremap + reg_count, val);

        /* Only map the core */
        while (val) {
            cpu = find_first_bit((void *)&val, 4);
            if (cpu != 4) {
                s->sw_coremap[reg_count] = cpu;
            }
            val = val >> 8;
            reg_count += 1;
        }
    } else if ((offset >= EXTIOI_NODETYPE_START) && (offset < EXTIOI_NODETYPE_END)) {
        stl_p((void *)s->nodetype + (offset - EXTIOI_NODETYPE_START), val);
    }
}

static void extioi_writel(void *opaque, hwaddr addr, uint64_t val,
                          unsigned size)
{
    loongarch_extioi *s = LOONGARCH_EXTIOI(opaque);
    int cpu, level, i, j, bits, ipnum, irqnum;
    uint64_t offset, old_data, reg_count;

    offset = addr & 0xffff;
    trace_loongarch_extioi_writel(size, (uint32_t)addr, val);

    if ((offset >= EXTIOI_ENABLE_START) && (offset < EXTIOI_ENABLE_END)) {
        reg_count = (offset - EXTIOI_ENABLE_START);
        old_data = ldq_p((void *)s->enable + reg_count);
        if (old_data != val) {
            stq_p((void *)s->enable + reg_count, val);
            old_data = old_data ^ val;

            bits = size * 8;
            while ((i = find_first_bit((void *)&old_data, bits)) != bits) {
                level = test_bit(i, (unsigned long *)&val);
                extioi_update_irq(s, i + reg_count * 8, level);
                clear_bit(i, (void *)&old_data);
            }
        }
    } else if ((offset >= EXTIOI_BOUNCE_START) && (offset < EXTIOI_BOUNCE_END)) {
        stq_p((void *)s->bounce + (offset - EXTIOI_BOUNCE_START), val);
    } else if ((offset >= EXTIOI_COREISR_START) && (offset < EXTIOI_COREISR_END)) {
        reg_count = ((offset - EXTIOI_COREISR_START) & 0x1f);
        cpu = ((offset - EXTIOI_COREISR_START) >> 8) & 0x3;

        /* core_ext_ioisr */
        old_data = ldq_p((void *)s->coreisr[cpu] + reg_count);
        stq_p((void *)s->coreisr[cpu] + reg_count, (old_data & ~val));

        if (old_data != (old_data & ~val)) {
            bits = size * 8;
            while ((i = find_first_bit((void *)&val, bits)) != bits) {
                j = test_bit(i, (unsigned long *)&old_data);
                if (j) {
                    extioi_update_irq(s, i + reg_count * 8, 0);
                }
                clear_bit(i, (void *)&val);
            }
        }
    } else if ((offset >= EXTIOI_IPMAP_START) && (offset < EXTIOI_IPMAP_END)) {
        /* Drop arch.core_ip_mask use s->ipmap */
        reg_count = (offset - EXTIOI_IPMAP_START);
        stq_p((void *)&s->ipmap + reg_count, val);

        /* Routing in groups of 32 interrupt */
        while (val) {
            ipnum = find_first_bit((void *)&val, 4);
            for (i = 0; i < 32; i++) {
                irqnum = reg_count * 32 + i;
                if (ipnum != 4) {
                    s->sw_ipmap[irqnum] = ipnum;
                } else {
                    s->sw_ipmap[irqnum] = 0;
                }
            }
            val = val >> 8;
            reg_count += 1;
        }
    } else if ((offset >= EXTIOI_COREMAP_START) && (offset < EXTIOI_COREMAP_END)) {
        reg_count = (offset - EXTIOI_COREMAP_START) / 8;
        stq_p((void *)s->coremap + reg_count, val);

        /* Only map the core */
        while (val) {
            cpu = find_first_bit((void *)&val, 4);
            if (cpu != 4) {
                s->sw_coremap[reg_count] = cpu;
            }
            val = val >> 8;
            reg_count += 1;
        }
    } else if ((offset >= EXTIOI_NODETYPE_START) && (offset < EXTIOI_NODETYPE_END)) {
        stq_p((void *)s->nodetype + (offset - EXTIOI_NODETYPE_START), val);
    }
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
        extioi_writeb(opaque, addr, value, size);
        break;
    case 4:
        extioi_writew(opaque, addr, value, size);
        break;
    case 8:
        extioi_writel(opaque, addr, value, size);
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
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void loongarch_extioi_realize(DeviceState *dev, Error **errp)
{
    LoongArchMachineState *lams = LOONGARCH_MACHINE(qdev_get_machine());
    MachineState *ms = MACHINE(lams);
    loongarch_extioi *p = LOONGARCH_EXTIOI(dev);
    int i, cpu, pin;

    qdev_init_gpio_in(dev, extioi_setirq, EXTIOI_IRQS);

    for (i = 0; i < EXTIOI_IRQS; i++) {
        sysbus_init_irq(SYS_BUS_DEVICE(dev), &p->irq[i]);
    }

    memory_region_init_io(&p->mmio, OBJECT(p), &extioi_ops, p,
                          TYPE_LOONGARCH_EXTIOI, 0x900);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &p->mmio);

    for (cpu = 0; cpu < ms->smp.cpus; cpu++) {
        for (pin = 0; pin < LS3A_INTC_IP; pin++) {
            qdev_init_gpio_out(dev, &p->parent_irq[cpu][pin], 1);
        }
    }
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
        VMSTATE_UINT64_ARRAY(enable, loongarch_extioi,
                             EXTIOI_IRQS_BITMAP_SIZE / 8),
        VMSTATE_UINT64_ARRAY(bounce, loongarch_extioi,
                             EXTIOI_IRQS_BITMAP_SIZE / 8),
        VMSTATE_UINT64_2DARRAY(coreisr, loongarch_extioi,
                               MAX_CORES, EXTIOI_IRQS_BITMAP_SIZE / 8),
        VMSTATE_UINT64(ipmap, loongarch_extioi),
        VMSTATE_UINT64_ARRAY(coremap, loongarch_extioi,
                             EXTIOI_IRQS_COREMAP_SIZE / 8),
        VMSTATE_UINT64_ARRAY(nodetype, loongarch_extioi,
                             EXTIOI_IRQS_NODETYPE_SIZE / 4),
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
