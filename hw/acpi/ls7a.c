/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * LoongArch ACPI implementation
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 */

#include "qemu/osdep.h"
#include "sysemu/sysemu.h"
#include "hw/hw.h"
#include "hw/irq.h"
#include "sysemu/reset.h"
#include "sysemu/runstate.h"
#include "hw/acpi/acpi.h"
#include "hw/acpi/ls7a.h"
#include "hw/nvram/fw_cfg.h"
#include "qemu/config-file.h"
#include "qapi/opts-visitor.h"
#include "qapi/qapi-events-run-state.h"
#include "qapi/error.h"
#include "hw/pci-host/ls7a.h"
#include "hw/mem/pc-dimm.h"
#include "hw/mem/nvdimm.h"
#include "migration/vmstate.h"

static void ls7a_pm_update_sci_fn(ACPIREGS *regs)
{
    LS7APMState *pm = container_of(regs, LS7APMState, acpi_regs);
    acpi_update_sci(&pm->acpi_regs, pm->irq);
}

static uint64_t ls7a_gpe_readb(void *opaque, hwaddr addr, unsigned width)
{
    LS7APMState *pm = opaque;
    return acpi_gpe_ioport_readb(&pm->acpi_regs, addr);
}

static void ls7a_gpe_writeb(void *opaque, hwaddr addr, uint64_t val,
                            unsigned width)
{
    LS7APMState *pm = opaque;
    acpi_gpe_ioport_writeb(&pm->acpi_regs, addr, val);
    acpi_update_sci(&pm->acpi_regs, pm->irq);
}

static const MemoryRegionOps ls7a_gpe_ops = {
    .read = ls7a_gpe_readb,
    .write = ls7a_gpe_writeb,
    .valid.min_access_size = 1,
    .valid.max_access_size = 8,
    .impl.min_access_size = 1,
    .impl.max_access_size = 1,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

#define VMSTATE_GPE_ARRAY(_field, _state)                            \
 {                                                                   \
     .name       = (stringify(_field)),                              \
     .version_id = 0,                                                \
     .num        = ACPI_GPE0_LEN,                                    \
     .info       = &vmstate_info_uint8,                              \
     .size       = sizeof(uint8_t),                                  \
     .flags      = VMS_ARRAY | VMS_POINTER,                          \
     .offset     = vmstate_offset_pointer(_state, _field, uint8_t),  \
 }

static uint64_t ls7a_reset_readw(void *opaque, hwaddr addr, unsigned width)
{
    return 0;
}

static void ls7a_reset_writew(void *opaque, hwaddr addr, uint64_t val,
                              unsigned width)
{
    if (val & 1) {
        qemu_system_reset_request(SHUTDOWN_CAUSE_GUEST_RESET);
        return;
    }
}

static const MemoryRegionOps ls7a_reset_ops = {
    .read = ls7a_reset_readw,
    .write = ls7a_reset_writew,
    .valid.min_access_size = 4,
    .valid.max_access_size = 4,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

const VMStateDescription vmstate_ls7a_pm = {
    .name = "ls7a_pm",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT16(acpi_regs.pm1.evt.sts, LS7APMState),
        VMSTATE_UINT16(acpi_regs.pm1.evt.en, LS7APMState),
        VMSTATE_UINT16(acpi_regs.pm1.cnt.cnt, LS7APMState),
        VMSTATE_TIMER_PTR(acpi_regs.tmr.timer, LS7APMState),
        VMSTATE_INT64(acpi_regs.tmr.overflow_time, LS7APMState),
        VMSTATE_GPE_ARRAY(acpi_regs.gpe.sts, LS7APMState),
        VMSTATE_GPE_ARRAY(acpi_regs.gpe.en, LS7APMState),
        VMSTATE_END_OF_LIST()
    },
};

static inline int64_t acpi_pm_tmr_get_clock(void)
{
    return muldiv64(qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL), PM_TIMER_FREQUENCY,
                    NANOSECONDS_PER_SECOND);
}

static uint32_t acpi_pm_tmr_get(ACPIREGS *ar)
{
    uint32_t d = acpi_pm_tmr_get_clock();
    return d & 0xffffff;
}

static void acpi_pm_tmr_timer(void *opaque)
{
    ACPIREGS *ar = opaque;
    qemu_system_wakeup_request(QEMU_WAKEUP_REASON_PMTIMER, NULL);
    ar->tmr.update_sci(ar);
}

static uint64_t acpi_pm_tmr_read(void *opaque, hwaddr addr, unsigned width)
{
    return acpi_pm_tmr_get(opaque);
}

static void acpi_pm_tmr_write(void *opaque, hwaddr addr, uint64_t val,
                              unsigned width)
{
}

static const MemoryRegionOps acpi_pm_tmr_ops = {
    .read = acpi_pm_tmr_read,
    .write = acpi_pm_tmr_write,
    .valid.min_access_size = 4,
    .valid.max_access_size = 4,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void ls7a_pm_tmr_init(ACPIREGS *ar, acpi_update_sci_fn update_sci,
                             MemoryRegion *parent, uint64_t offset)
{
    ar->tmr.update_sci = update_sci;
    ar->tmr.timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, acpi_pm_tmr_timer, ar);
    memory_region_init_io(&ar->tmr.io, memory_region_owner(parent),
                          &acpi_pm_tmr_ops, ar, "acpi-tmr", 4);
    memory_region_add_subregion(parent, offset, &ar->tmr.io);
}

static void acpi_pm1_evt_write_sts(ACPIREGS *ar, uint16_t val)
{
    uint16_t pm1_sts = acpi_pm1_evt_get_sts(ar);
    if (pm1_sts & val & ACPI_BITMASK_TIMER_STATUS) {
        /* if TMRSTS is reset, then compute the new overflow time */
        acpi_pm_tmr_calc_overflow_time(ar);
    }
    ar->pm1.evt.sts &= ~val;
}

static uint64_t acpi_pm_evt_read(void *opaque, hwaddr addr, unsigned width)
{
    ACPIREGS *ar = opaque;
    switch (addr) {
    case 0:
        return acpi_pm1_evt_get_sts(ar);
    case 4:
        return ar->pm1.evt.en;
    default:
        return 0;
    }
}

static void acpi_pm1_evt_write_en(ACPIREGS *ar, uint16_t val)
{
    ar->pm1.evt.en = val;
    qemu_system_wakeup_enable(QEMU_WAKEUP_REASON_RTC,
                              val & ACPI_BITMASK_RT_CLOCK_ENABLE);
    qemu_system_wakeup_enable(QEMU_WAKEUP_REASON_PMTIMER,
                              val & ACPI_BITMASK_TIMER_ENABLE);
}

static void acpi_pm_evt_write(void *opaque, hwaddr addr, uint64_t val,
                              unsigned width)
{
    ACPIREGS *ar = opaque;
    switch (addr) {
    case 0:
        acpi_pm1_evt_write_sts(ar, val);
        ar->pm1.evt.update_sci(ar);
        break;
    case 4:
        acpi_pm1_evt_write_en(ar, val);
        ar->pm1.evt.update_sci(ar);
        break;
    }
}

static const MemoryRegionOps acpi_pm_evt_ops = {
    .read = acpi_pm_evt_read,
    .write = acpi_pm_evt_write,
    .valid.min_access_size = 1,
    .valid.max_access_size = 4,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void ls7a_pm1_evt_init(ACPIREGS *ar, acpi_update_sci_fn update_sci,
                              MemoryRegion *parent, uint64_t offset)
{
    ar->pm1.evt.update_sci = update_sci;
    memory_region_init_io(&ar->pm1.evt.io, memory_region_owner(parent),
                          &acpi_pm_evt_ops, ar, "acpi-evt", 8);
    memory_region_add_subregion(parent, offset, &ar->pm1.evt.io);
}

static uint64_t acpi_pm_cnt_read(void *opaque, hwaddr addr, unsigned width)
{
    ACPIREGS *ar = opaque;
    return ar->pm1.cnt.cnt;
}

/* ACPI PM1aCNT */
static void acpi_pm1_cnt_write(ACPIREGS *ar, uint16_t val)
{
    ar->pm1.cnt.cnt = val & ~(ACPI_BITMASK_SLEEP_ENABLE);

    if (val & ACPI_BITMASK_SLEEP_ENABLE) {
        /* Change suspend type */
        uint16_t sus_typ = (val >> 10) & 7;
        switch (sus_typ) {
        /* Not support s3 s4 yet */
        case 7: /* Soft power off */
            qemu_system_shutdown_request(SHUTDOWN_CAUSE_GUEST_SHUTDOWN);
            break;
        default:
            break;
        }
    }
}

static void acpi_pm_cnt_write(void *opaque, hwaddr addr, uint64_t val,
                              unsigned width)
{
    acpi_pm1_cnt_write(opaque, val);
}

static const MemoryRegionOps acpi_pm_cnt_ops = {
    .read = acpi_pm_cnt_read,
    .write = acpi_pm_cnt_write,
    .valid.min_access_size = 1,
    .valid.max_access_size = 4,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void acpi_notify_wakeup(Notifier *notifier, void *data)
{
    ACPIREGS *ar = container_of(notifier, ACPIREGS, wakeup);
    WakeupReason *reason = data;

    switch (*reason) {
    case QEMU_WAKEUP_REASON_RTC:
        ar->pm1.evt.sts |=
            (ACPI_BITMASK_WAKE_STATUS | ACPI_BITMASK_RT_CLOCK_STATUS);
        break;
    case QEMU_WAKEUP_REASON_PMTIMER:
        ar->pm1.evt.sts |=
            (ACPI_BITMASK_WAKE_STATUS | ACPI_BITMASK_TIMER_STATUS);
        break;
    case QEMU_WAKEUP_REASON_OTHER:
        /*
         * ACPI_BITMASK_WAKE_STATUS should be set on resume.
         * Pretend that resume was caused by power button
         */
        ar->pm1.evt.sts |=
            (ACPI_BITMASK_WAKE_STATUS | ACPI_BITMASK_POWER_BUTTON_STATUS);
        break;
    default:
        break;
    }
}

static void ls7a_pm1_cnt_init(ACPIREGS *ar, MemoryRegion *parent,
                              uint64_t offset)
{
    ar->wakeup.notify = acpi_notify_wakeup;
    qemu_register_wakeup_notifier(&ar->wakeup);
    memory_region_init_io(&ar->pm1.cnt.io, memory_region_owner(parent),
                          &acpi_pm_cnt_ops, ar, "acpi-cnt", 4);
    memory_region_add_subregion(parent, offset, &ar->pm1.cnt.io);
}

static void ls7a_pm_reset(DeviceState *d)
{
    LS7APMState *pm = LS7A_PM(d);

    acpi_pm1_evt_reset(&pm->acpi_regs);
    acpi_pm1_cnt_reset(&pm->acpi_regs);
    acpi_pm_tmr_reset(&pm->acpi_regs);
    acpi_gpe_reset(&pm->acpi_regs);

    acpi_update_sci(&pm->acpi_regs, pm->irq);
}

static void pm_powerdown_req(Notifier *n, void *opaque)
{
    LS7APMState *pm = container_of(n, LS7APMState, powerdown_notifier);

    acpi_pm1_evt_power_down(&pm->acpi_regs);
}

void ls7a_pm_init(DeviceState *ls7a_pm, qemu_irq pm_irq)
{
    LS7APMState *pm = LS7A_PM(ls7a_pm);
    pm->irq = pm_irq;
}

static void ls7a_pm_realize(DeviceState *dev, Error **errp)
{
    LS7APMState *pm = LS7A_PM(dev);
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);

    /*
     * ls7a board acpi hardware info, including
     * acpi system io base address
     * acpi gpe length
     * acpi sci irq number
     */

    memory_region_init(&pm->iomem, OBJECT(pm), "ls7a_pm", ACPI_IO_SIZE);
    sysbus_init_mmio(sbd, &pm->iomem);

    ls7a_pm_tmr_init(&pm->acpi_regs, ls7a_pm_update_sci_fn,
                     &pm->iomem, LS7A_PM_TMR_BLK);
    ls7a_pm1_evt_init(&pm->acpi_regs, ls7a_pm_update_sci_fn,
                      &pm->iomem, LS7A_PM_EVT_BLK);
    ls7a_pm1_cnt_init(&pm->acpi_regs, &pm->iomem, LS7A_PM_CNT_BLK);

    acpi_gpe_init(&pm->acpi_regs, ACPI_GPE0_LEN);
    memory_region_init_io(&pm->iomem_gpe, OBJECT(pm), &ls7a_gpe_ops, pm,
                          "acpi-gpe0", ACPI_GPE0_LEN);
    sysbus_init_mmio(sbd, &pm->iomem_gpe);

    memory_region_init_io(&pm->iomem_reset, OBJECT(pm),
                          &ls7a_reset_ops, pm, "acpi-reset", 4);
    sysbus_init_mmio(sbd, &pm->iomem_reset);

    pm->powerdown_notifier.notify = pm_powerdown_req;
    qemu_register_powerdown_notifier(&pm->powerdown_notifier);
}

static void ls7a_pm_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = ls7a_pm_realize;
    dc->reset = ls7a_pm_reset;
    dc->desc = "PM";
    dc->vmsd = &vmstate_ls7a_pm;
}

static const TypeInfo ls7a_pm_info = {
    .name          = TYPE_LS7A_PM,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(LS7APMState),
    .class_init    = ls7a_pm_class_init,
};

static void ls7a_pm_register_types(void)
{
    type_register_static(&ls7a_pm_info);
}

type_init(ls7a_pm_register_types)
