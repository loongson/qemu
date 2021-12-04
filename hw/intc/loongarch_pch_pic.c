/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * QEMU Loongson 7A1000 I/O interrupt controller.
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 */

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "hw/loongarch/loongarch.h"
#include "hw/irq.h"
#include "hw/intc/loongarch_pch_pic.h"
#include "migration/vmstate.h"
#include "trace.h"

#define for_each_set_bit(bit, addr, size) \
         for ((bit) = find_first_bit((addr), (size));            \
              (bit) < (size);                                    \
              (bit) = find_next_bit((addr), (size), (bit) + 1))

static void pch_pic_update_irq(loongarch_pch_pic *s, uint64_t mask, int level)
{
    int i;
    uint64_t val;
    val = mask & s->intirr & (~s->int_mask);

    for_each_set_bit(i, &val, 64) {
        if (level == 1) {
            if ((s->intisr & (0x1ULL << i)) == 0) {
                s->intisr |= 1ULL << i;
                qemu_set_irq(s->parent_irq[s->htmsi_vector[i]], 1);
            }
        } else if (level == 0) {
            if (s->intisr & (0x1ULL << i)) {
                s->intisr &= ~(0x1ULL << i);
                qemu_set_irq(s->parent_irq[s->htmsi_vector[i]], 0);
            }
        }
    }
}

static void pch_pic_irq_handler(void *opaque, int irq, int level)
{
    loongarch_pch_pic *s = LOONGARCH_PCH_PIC(opaque);

    assert(irq < PCH_PIC_IRQ_NUM);
    uint64_t mask = 1ULL << irq;

    trace_pch_pic_irq_handler(s->intedge, irq, level);

    if (s->intedge & mask) {
        /* Edge triggered */
        if (level) {
            if ((s->last_intirr & mask) == 0) {
                s->intirr |= mask;
            }
            s->last_intirr |= mask;
        } else {
            s->last_intirr &= ~mask;
        }
    } else {
        /* Level triggered */
        if (level) {
            s->intirr |= mask;
            s->last_intirr |= mask;
        } else {
            s->intirr &= ~mask;
            s->last_intirr &= ~mask;
        }

    }
    pch_pic_update_irq(s, mask, level);
}

static uint64_t loongarch_pch_pic_reg_read(void *opaque, hwaddr addr,
                                           unsigned size)
{
    loongarch_pch_pic *s = LOONGARCH_PCH_PIC(opaque);
    uint64_t val = 0;
    uint32_t offset = addr & 0xfff;
    int64_t offset_tmp;

    if (size == 8) {
        switch (offset) {
        case PCH_PIC_INT_ID_OFFSET:
            val = (PCH_PIC_INT_ID_NUM << 32) | PCH_PIC_INT_ID_VAL;
            break;
        case PCH_PIC_INT_MASK_OFFSET:
            val = s->int_mask;
            break;
        case PCH_PIC_INT_STATUS_OFFSET:
            val = s->intisr & (~s->int_mask);
            break;
        case PCH_PIC_INT_EDGE_OFFSET:
            val = s->intedge;
            break;
        case PCH_PIC_INT_POL_OFFSET:
            val = s->int_polarity;
            break;
        case PCH_PIC_HTMSI_EN_OFFSET...PCH_PIC_HTMSI_EN_END:
            val = s->htmsi_en;
            break;
        case PCH_PIC_AUTO_CTRL0_OFFSET...PCH_PIC_AUTO_CTRL0_END:
        case PCH_PIC_AUTO_CTRL1_OFFSET...PCH_PIC_AUTO_CTRL1_END:
            break;
        default:
            break;
        }
    } else if (size == 4) {
        switch (offset) {
        case PCH_PIC_INT_ID_OFFSET:
            val = PCH_PIC_INT_ID_VAL;
            break;
        case PCH_PIC_INT_ID_OFFSET + 4:
            val = PCH_PIC_INT_ID_NUM;
            break;
        case PCH_PIC_INT_MASK_OFFSET...PCH_PIC_INT_MASK_END:
            val = ldl_p((void *)&s->int_mask +
                        (offset - PCH_PIC_INT_MASK_OFFSET));
            break;
        case PCH_PIC_INT_STATUS_OFFSET...PCH_PIC_INT_STATUS_END:
            val = ldl_p((void *)&s->intisr +
                        (offset - PCH_PIC_INT_STATUS_OFFSET)) & (~s->int_mask);
            break;
        case PCH_PIC_INT_EDGE_OFFSET...PCH_PIC_INT_EDGE_END:
            val = ldl_p((void *)&s->intedge +
                        (offset - PCH_PIC_INT_EDGE_OFFSET));
            break;
        case PCH_PIC_INT_POL_OFFSET...PCH_PIC_INT_POL_END:
            val = ldl_p((void *)&s->int_polarity +
                        (offset - PCH_PIC_INT_POL_OFFSET));
            break;
        case PCH_PIC_HTMSI_EN_OFFSET...PCH_PIC_HTMSI_EN_END:
            val = ldl_p((void *)&s->htmsi_en +
                        (offset - PCH_PIC_HTMSI_EN_OFFSET));
            break;
        case PCH_PIC_AUTO_CTRL0_OFFSET...PCH_PIC_AUTO_CTRL0_END:
        case PCH_PIC_AUTO_CTRL1_OFFSET...PCH_PIC_AUTO_CTRL1_END:
            break;
        default:
            break;
        }
    } else if (size == 1) {
        if (offset >= PCH_PIC_HTMSI_VEC_OFFSET) {
            offset_tmp = offset - PCH_PIC_HTMSI_VEC_OFFSET;
            if (offset_tmp >= 0 && offset_tmp < 64) {
                val = s->htmsi_vector[offset_tmp];
            }
        } else if (offset >=  PCH_PIC_ROUTE_ENTRY_OFFSET) {
            offset_tmp = offset - PCH_PIC_ROUTE_ENTRY_OFFSET;
            if (offset_tmp >= 0 && offset_tmp < 64) {
                val = s->route_entry[offset_tmp];
            }
        }
    }

    trace_loongarch_pch_pic_read(size, (uint32_t)addr, val);
    return val;
}

static void loongarch_pch_pic_reg_write(void *opaque, hwaddr addr,
                                        uint64_t data, unsigned size)
{
    loongarch_pch_pic *s = LOONGARCH_PCH_PIC(opaque);
    int32_t offset_tmp;
    uint32_t offset, old;
    offset = addr & 0xfff;

    trace_loongarch_pch_pic_write(size, (uint32_t)addr, data);

    if (size == 8) {
        switch (offset) {
        case PCH_PIC_INT_MASK_OFFSET:
            old = s->int_mask;
            s->int_mask = data;
            if (old & ~data) {
                pch_pic_update_irq(s, (old & ~data), 1);
            } else if (~old & data) {
                pch_pic_update_irq(s, (~old & data), 0);
            }
            break;
        case PCH_PIC_INT_STATUS_OFFSET:
            s->intisr = data;
            break;
        case PCH_PIC_INT_EDGE_OFFSET:
            s->intedge = data;
            break;
        case PCH_PIC_INT_CLEAR_OFFSET:
            s->intirr &= (~(data & s->intedge));
            pch_pic_update_irq(s, data, 0);
            s->intisr &= (~data);
            break;
        case PCH_PIC_INT_POL_OFFSET:
            s->int_polarity = data;
            break;
        case PCH_PIC_HTMSI_EN_OFFSET:
            s->htmsi_en = data;
            break;
        case PCH_PIC_AUTO_CTRL0_OFFSET:
        case PCH_PIC_AUTO_CTRL1_OFFSET:
            break;
        default:
            break;
        }
    } else if (size == 4) {
        switch (offset) {
        case PCH_PIC_INT_MASK_OFFSET...PCH_PIC_INT_MASK_END:
            offset -= PCH_PIC_INT_MASK_OFFSET;
            old = ldl_p((void *)&s->int_mask + offset);
            stl_p((void *)&s->int_mask + offset, data);

            if (old & ~data) {
                pch_pic_update_irq(s, (old & ~data), 1);
            } else if (~old & data) {
                pch_pic_update_irq(s, (~old & data), 0);
            }
            break;
        case PCH_PIC_INT_STATUS_OFFSET...PCH_PIC_INT_STATUS_END:
            stl_p((void *)&s->intisr + (offset - PCH_PIC_INT_STATUS_OFFSET),
                  data);
            break;
        case PCH_PIC_INT_EDGE_OFFSET...PCH_PIC_INT_EDGE_END:
            stl_p((void *)&s->intedge + (offset - PCH_PIC_INT_EDGE_OFFSET),
                  data);
            break;
        case PCH_PIC_INT_CLEAR_OFFSET...PCH_PIC_INT_CLEAR_END:
            old = s->intirr & (~(data & s->intedge));
            stl_p((void *)&s->intirr + (offset - PCH_PIC_INT_CLEAR_OFFSET),
                  old);
            pch_pic_update_irq(s, data, 0);
            old = s->intisr & (~data);
            stl_p((void *)&s->intisr + (offset - PCH_PIC_INT_CLEAR_OFFSET),
                  old);
            break;
        case PCH_PIC_INT_POL_OFFSET...PCH_PIC_INT_POL_END:
            stl_p((void *)&s->int_polarity + (offset - PCH_PIC_INT_POL_OFFSET),
                  data);
            break;
        case PCH_PIC_HTMSI_EN_OFFSET...PCH_PIC_HTMSI_EN_END:
            stl_p((void *)&s->htmsi_en + (offset - PCH_PIC_HTMSI_EN_OFFSET),
                  data);
            break;
        case PCH_PIC_AUTO_CTRL0_OFFSET...PCH_PIC_AUTO_CTRL0_END:
        case PCH_PIC_AUTO_CTRL1_OFFSET...PCH_PIC_AUTO_CTRL1_END:
            break;
        default:
            break;
        }
    } else if (size == 1) {
        if (offset >= PCH_PIC_HTMSI_VEC_OFFSET) {
            offset_tmp = offset - PCH_PIC_HTMSI_VEC_OFFSET;
            if (offset_tmp >= 0 && offset_tmp < 64) {
                s->htmsi_vector[offset_tmp] = (uint8_t)(data & 0xff);
            }
        } else if (offset >=  PCH_PIC_ROUTE_ENTRY_OFFSET) {
            offset_tmp = offset - PCH_PIC_ROUTE_ENTRY_OFFSET;
            if (offset_tmp >= 0 && offset_tmp < 64) {
                s->route_entry[offset_tmp] = (uint8_t)(data & 0xff);
            }
        }
    }
}

static const MemoryRegionOps loongarch_pch_pic_ops = {
    .read = loongarch_pch_pic_reg_read,
    .write = loongarch_pch_pic_reg_write,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 8,
    },
    .impl = {
        .min_access_size = 1,
        .max_access_size = 8,
    },
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void loongarch_pch_pic_reset(DeviceState *d)
{
    loongarch_pch_pic *s = LOONGARCH_PCH_PIC(d);
    int i;

    s->int_mask = -1ULL;
    s->htmsi_en = 0x0;
    s->intedge  = 0x0;
    s->intclr   = 0x0;
    s->auto_crtl0 = 0x0;
    s->auto_crtl1 = 0x0;
    for (i = 0; i < 64; i++) {
        s->route_entry[i] = 0x1;
        s->htmsi_vector[i] = 0x0;
    }
    s->intirr = 0x0;
    s->intisr = 0x0;
    s->last_intirr = 0x0;
    s->int_polarity = 0x0;
}

static void loongarch_pch_pic_init(Object *obj)
{
    loongarch_pch_pic *s = LOONGARCH_PCH_PIC(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);
    int i;

    memory_region_init_io(&s->iomem, obj, &loongarch_pch_pic_ops,
                          s, TYPE_LOONGARCH_PCH_PIC, 0x1000);
    sysbus_init_mmio(sbd, &s->iomem);

    for (i = 0; i < PCH_PIC_IRQ_NUM; i++) {
        sysbus_init_irq(sbd, &s->parent_irq[i]);
    }
    qdev_init_gpio_in(DEVICE(obj), pch_pic_irq_handler, PCH_PIC_IRQ_NUM);
}

static const VMStateDescription vmstate_loongarch_pch_pic = {
    .name = TYPE_LOONGARCH_PCH_PIC,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT64(int_mask, loongarch_pch_pic),
        VMSTATE_UINT64(htmsi_en, loongarch_pch_pic),
        VMSTATE_UINT64(intedge, loongarch_pch_pic),
        VMSTATE_UINT64(intclr, loongarch_pch_pic),
        VMSTATE_UINT64(auto_crtl0, loongarch_pch_pic),
        VMSTATE_UINT64(auto_crtl1, loongarch_pch_pic),
        VMSTATE_UINT8_ARRAY(route_entry, loongarch_pch_pic, 64),
        VMSTATE_UINT8_ARRAY(htmsi_vector, loongarch_pch_pic, 64),
        VMSTATE_UINT64(last_intirr, loongarch_pch_pic),
        VMSTATE_UINT64(intirr, loongarch_pch_pic),
        VMSTATE_UINT64(intisr, loongarch_pch_pic),
        VMSTATE_UINT64(int_polarity, loongarch_pch_pic),
        VMSTATE_END_OF_LIST()
    }
};

static void loongarch_pch_pic_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = loongarch_pch_pic_reset;
    dc->vmsd = &vmstate_loongarch_pch_pic;
}

static const TypeInfo loongarch_pch_pic_info = {
    .name          = TYPE_LOONGARCH_PCH_PIC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(loongarch_pch_pic),
    .instance_init = loongarch_pch_pic_init,
    .class_init    = loongarch_pch_pic_class_init,
};

static void loongarch_pch_pic_register_types(void)
{
    type_register_static(&loongarch_pch_pic_info);
}

type_init(loongarch_pch_pic_register_types)
