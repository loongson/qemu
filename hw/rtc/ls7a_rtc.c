/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Loongarch LS7A Real Time Clock emulation
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 */

#include "qemu/osdep.h"
#include "qemu-common.h"
#include "hw/sysbus.h"
#include "hw/irq.h"
#include "include/hw/register.h"
#include "qemu/timer.h"
#include "sysemu/sysemu.h"
#include "qemu/cutils.h"
#include "qemu/log.h"
#include "migration/vmstate.h"
#include "hw/misc/unimp.h"

#define SYS_TOYTRIM        0x20
#define SYS_TOYWRITE0      0x24
#define SYS_TOYWRITE1      0x28
#define SYS_TOYREAD0       0x2C
#define SYS_TOYREAD1       0x30
#define SYS_TOYMATCH0      0x34
#define SYS_TOYMATCH1      0x38
#define SYS_TOYMATCH2      0x3C
#define SYS_RTCCTRL        0x40
#define SYS_RTCTRIM        0x60
#define SYS_RTCWRTIE0      0x64
#define SYS_RTCREAD0       0x68
#define SYS_RTCMATCH0      0x6C
#define SYS_RTCMATCH1      0x70
#define SYS_RTCMATCH2      0x74

/*
 * Shift bits and filed mask
 */
#define TOY_MON_MASK   0x3f
#define TOY_DAY_MASK   0x1f
#define TOY_HOUR_MASK  0x1f
#define TOY_MIN_MASK   0x3f
#define TOY_SEC_MASK   0x3f
#define TOY_MSEC_MASK  0xf

#define TOY_MON_SHIFT  26
#define TOY_DAY_SHIFT  21
#define TOY_HOUR_SHIFT 16
#define TOY_MIN_SHIFT  10
#define TOY_SEC_SHIFT  4
#define TOY_MSEC_SHIFT 0

#define TOY_MATCH_YEAR_MASK  0x3f
#define TOY_MATCH_MON_MASK   0xf
#define TOY_MATCH_DAY_MASK   0x1f
#define TOY_MATCH_HOUR_MASK  0x1f
#define TOY_MATCH_MIN_MASK   0x3f
#define TOY_MATCH_SEC_MASK   0x3f

#define TOY_MATCH_YEAR_SHIFT 26
#define TOY_MATCH_MON_SHIFT  22
#define TOY_MATCH_DAY_SHIFT  17
#define TOY_MATCH_HOUR_SHIFT 12
#define TOY_MATCH_MIN_SHIFT  6
#define TOY_MATCH_SEC_SHIFT  0

#define TOY_ENABLE_BIT (1U << 11)

#define TYPE_LS7A_RTC "ls7a_rtc"
OBJECT_DECLARE_SIMPLE_TYPE(LS7A_RTCState, LS7A_RTC)

typedef struct LS7A_RTCState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    QEMUTimer *timer;
    /*
     * Needed to preserve the tick_count across migration, even if the
     * absolute value of the rtc_clock is different on the source and
     * destination.
     */
    int64_t offset;
    int64_t data;
    int64_t save_alarm_offset;
    int tidx;
    uint32_t toymatch[3];
    uint32_t toytrim;
    uint32_t cntrctl;
    uint32_t rtctrim;
    uint32_t rtccount;
    uint32_t rtcmatch[3];
    qemu_irq toy_irq;
} LS7A_RTCState;

enum {
    TOYEN = 1UL << 11,
    RTCEN = 1UL << 13,
};

static uint64_t ls7a_rtc_read(void *opaque, hwaddr addr, unsigned size)
{
    LS7A_RTCState *s = LS7A_RTC(opaque);
    struct tm tm;
    unsigned int val;

    val = 0;

    switch (addr) {
    case SYS_TOYREAD0:
        qemu_get_timedate(&tm, s->offset);
        val = (((tm.tm_mon + 1) & TOY_MON_MASK) << TOY_MON_SHIFT)
        | (((tm.tm_mday) & TOY_DAY_MASK) << TOY_DAY_SHIFT)
        | (((tm.tm_hour) & TOY_HOUR_MASK) << TOY_HOUR_SHIFT)
        | (((tm.tm_min) & TOY_MIN_MASK) << TOY_MIN_SHIFT)
        | (((tm.tm_sec) & TOY_SEC_MASK) << TOY_SEC_SHIFT) | 0x0;
        break;
    case SYS_TOYREAD1:
        qemu_get_timedate(&tm, s->offset);
        val = tm.tm_year;
        break;
    case SYS_TOYMATCH0:
        val = s->toymatch[0];
        break;
    case SYS_TOYMATCH1:
        val = s->toymatch[1];
        break;
    case SYS_TOYMATCH2:
        val = s->toymatch[2];
        break;
    case SYS_RTCCTRL:
        val = s->cntrctl;
        break;
    case SYS_RTCREAD0:
        val = s->rtccount;
        break;
    case SYS_RTCMATCH0:
        val = s->rtcmatch[0];
        break;
    case SYS_RTCMATCH1:
        val = s->rtcmatch[1];
        break;
    case SYS_RTCMATCH2:
        val = s->rtcmatch[2];
        break;
    default:
        val = 0;
        break;
    }
    return val;
}

static void ls7a_rtc_write(void *opaque, hwaddr addr,
                           uint64_t val, unsigned size)
{
    LS7A_RTCState *s = LS7A_RTC(opaque);
    struct tm tm;
    int64_t alarm_offset, year_diff, expire_time;

    switch (addr) {
    case SYS_TOYWRITE0:
        qemu_get_timedate(&tm, s->offset);
        tm.tm_sec = (val >> TOY_SEC_SHIFT) & TOY_SEC_MASK;
        tm.tm_min = (val >> TOY_MIN_SHIFT) & TOY_MIN_MASK;
        tm.tm_hour = (val >> TOY_HOUR_SHIFT) & TOY_HOUR_MASK;
        tm.tm_mday = ((val >> TOY_DAY_SHIFT) & TOY_DAY_MASK);
        tm.tm_mon = ((val >> TOY_MON_SHIFT) & TOY_MON_MASK) - 1;
        s->offset = qemu_timedate_diff(&tm);
        break;
    case SYS_TOYWRITE1:
        qemu_get_timedate(&tm, s->offset);
        tm.tm_year = val;
        s->offset = qemu_timedate_diff(&tm);
        break;
    case SYS_TOYMATCH0:
        s->toymatch[0] = val;
        qemu_get_timedate(&tm, s->offset);
        tm.tm_sec = (val >> TOY_MATCH_SEC_SHIFT) & TOY_MATCH_SEC_MASK;
        tm.tm_min = (val >> TOY_MATCH_MIN_SHIFT) & TOY_MATCH_MIN_MASK;
        tm.tm_hour = ((val >> TOY_MATCH_HOUR_SHIFT) & TOY_MATCH_HOUR_MASK);
        tm.tm_mday = ((val >> TOY_MATCH_DAY_SHIFT) & TOY_MATCH_DAY_MASK);
        tm.tm_mon = ((val >> TOY_MATCH_MON_SHIFT) & TOY_MATCH_MON_MASK) - 1;
        year_diff = ((val >> TOY_MATCH_YEAR_SHIFT) & TOY_MATCH_YEAR_MASK);
        year_diff = year_diff - (tm.tm_year & TOY_MATCH_YEAR_MASK);
        tm.tm_year = tm.tm_year + year_diff;
        alarm_offset = qemu_timedate_diff(&tm) - s->offset;
        if ((alarm_offset < 0) && (alarm_offset > -5)) {
                alarm_offset = 0;
        }
        expire_time = qemu_clock_get_ms(rtc_clock);
        expire_time += ((alarm_offset * 1000) + 100);
        timer_mod(s->timer, expire_time);
        break;
    case SYS_TOYMATCH1:
        s->toymatch[1] = val;
        break;
    case SYS_TOYMATCH2:
        s->toymatch[2] = val;
        break;
    case SYS_RTCCTRL:
        s->cntrctl = val;
        break;
    case SYS_RTCWRTIE0:
        s->rtccount = val;
        break;
    case SYS_RTCMATCH0:
        s->rtcmatch[0] = val;
        break;
    case SYS_RTCMATCH1:
        val = s->rtcmatch[1];
        break;
    case SYS_RTCMATCH2:
        val = s->rtcmatch[2];
        break;
    default:
        break;
    }
}

static const MemoryRegionOps ls7a_rtc_ops = {
    .read = ls7a_rtc_read,
    .write = ls7a_rtc_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },

};

static void toy_timer(void *opaque)
{
    LS7A_RTCState *s = LS7A_RTC(opaque);

    if (s->cntrctl & TOY_ENABLE_BIT) {
            qemu_irq_pulse(s->toy_irq);
    }
}

static void ls7a_rtc_realize(DeviceState *dev, Error **errp)
{
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    LS7A_RTCState *d = LS7A_RTC(sbd);
    memory_region_init_io(&d->iomem, NULL, &ls7a_rtc_ops,
                    (void *)d, "ls7a_rtc", 0x100);

    sysbus_init_irq(sbd, &d->toy_irq);

    sysbus_init_mmio(sbd, &d->iomem);
    d->timer = timer_new_ms(rtc_clock, toy_timer, d);
    timer_mod(d->timer, qemu_clock_get_ms(rtc_clock) + 100);
    d->offset = 0;

    create_unimplemented_device("mmio fallback 1", 0x10013ffc, 0x4);
}

static int ls7a_rtc_pre_save(void *opaque)
{
    LS7A_RTCState *s = LS7A_RTC(opaque);
    struct tm tm;
    int64_t year_diff, value;

    value = s->toymatch[0];
    qemu_get_timedate(&tm, s->offset);
    tm.tm_sec = (value >> TOY_MATCH_SEC_SHIFT) & TOY_MATCH_SEC_MASK;
    tm.tm_min = (value >> TOY_MATCH_MIN_SHIFT) & TOY_MATCH_MIN_MASK;
    tm.tm_hour = ((value >> TOY_MATCH_HOUR_SHIFT) & TOY_MATCH_HOUR_MASK);
    tm.tm_mday = ((value >> TOY_MATCH_DAY_SHIFT) & TOY_MATCH_DAY_MASK);
    tm.tm_mon = ((value >> TOY_MATCH_MON_SHIFT) & TOY_MATCH_MON_MASK) - 1;
    year_diff = ((value >> TOY_MATCH_YEAR_SHIFT) & TOY_MATCH_YEAR_MASK);
    year_diff = year_diff - (tm.tm_year & TOY_MATCH_YEAR_MASK);
    tm.tm_year = tm.tm_year + year_diff;
    s->save_alarm_offset = qemu_timedate_diff(&tm) - s->offset;

    return 0;
}

static int ls7a_rtc_post_load(void *opaque, int version_id)
{
    LS7A_RTCState *s = LS7A_RTC(opaque);
    int64_t expire_time;

    expire_time = qemu_clock_get_ms(rtc_clock) + (s->save_alarm_offset * 1000);
    timer_mod(s->timer, expire_time);

    return 0;
}

static const VMStateDescription vmstate_ls7a_rtc = {
    .name = "ls7a_rtc",
    .version_id = 1,
    .minimum_version_id = 1,
    .pre_save = ls7a_rtc_pre_save,
    .post_load = ls7a_rtc_post_load,
    .fields = (VMStateField[]) {
        VMSTATE_INT64(offset, LS7A_RTCState),
        VMSTATE_INT64(save_alarm_offset, LS7A_RTCState),
        VMSTATE_UINT32(toymatch[0], LS7A_RTCState),
        VMSTATE_UINT32(cntrctl, LS7A_RTCState),
        VMSTATE_END_OF_LIST()
    }
};

static void ls7a_rtc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->vmsd = &vmstate_ls7a_rtc;
    dc->realize = ls7a_rtc_realize;
    dc->desc = "ls7a rtc";
}

static const TypeInfo ls7a_rtc_info = {
    .name          = TYPE_LS7A_RTC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(LS7A_RTCState),
    .class_init    = ls7a_rtc_class_init,
};

static void ls7a_rtc_register_types(void)
{
    type_register_static(&ls7a_rtc_info);
}

type_init(ls7a_rtc_register_types)
