/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Loongarch LS7A Real Time Clock emulation
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 */

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "hw/irq.h"
#include "include/hw/register.h"
#include "qemu/timer.h"
#include "sysemu/sysemu.h"
#include "qemu/cutils.h"
#include "qemu/log.h"
#include "migration/vmstate.h"
#include "hw/misc/unimp.h"
#include "sysemu/rtc.h"
#include "hw/registerfields.h"

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

#define LS7A_RTC_FREQ     32768
#define TIMER_NUMS        3
/*
 * Shift bits and filed mask
 */

FIELD(TOY, MON, 26, 6)
FIELD(TOY, DAY, 21, 5)
FIELD(TOY, HOUR, 16, 5)
FIELD(TOY, MIN, 10, 6)
FIELD(TOY, SEC, 4, 6)
FIELD(TOY, MSEC, 0, 4)

FIELD(TOY_MATCH, YEAR, 26, 6)
FIELD(TOY_MATCH, MON, 22, 4)
FIELD(TOY_MATCH, DAY, 17, 5)
FIELD(TOY_MATCH, HOUR, 12, 5)
FIELD(TOY_MATCH, MIN, 6, 6)
FIELD(TOY_MATCH, SEC, 0, 6)

FIELD(RTC_CTRL, RTCEN, 13, 1)
FIELD(RTC_CTRL, TOYEN, 11, 1)
FIELD(RTC_CTRL, EO, 8, 1)

#define TYPE_LS7A_RTC "ls7a_rtc"
OBJECT_DECLARE_SIMPLE_TYPE(LS7ARtcState, LS7A_RTC)

typedef struct LS7ARtcTimer {
    QEMUTimer *timer;
    int64_t save_offset;
    int64_t enable_offset;
    int32_t flag;
    LS7ARtcState *d;
} LS7ARtcTimer;

struct LS7ARtcState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    /*
     * Needed to preserve the tick_count across migration, even if the
     * absolute value of the rtc_clock is different on the source and
     * destination.
     */
    int64_t offset_toy;
    int64_t offset_rtc;
    int64_t data;
    int tidx;
    uint32_t toymatch[3];
    uint32_t toytrim;
    uint32_t cntrctl;
    uint32_t rtctrim;
    uint32_t rtccount;
    uint32_t rtcmatch[3];
    LS7ARtcTimer toy_timer[TIMER_NUMS];
    LS7ARtcTimer rtc_timer[TIMER_NUMS];
    qemu_irq irq;
};

static int64_t ls7a_rtc_ticks(void)
{
    return qemu_clock_get_ms(rtc_clock) * LS7A_RTC_FREQ / 1000;
}

static uint64_t ls7a_rtc_read(void *opaque, hwaddr addr, unsigned size)
{
    LS7ARtcState *s = LS7A_RTC(opaque);
    struct tm tm;
    int val = 0;

    switch (addr) {
    case SYS_TOYREAD0:
        qemu_get_timedate(&tm, s->offset_toy);
        val = FIELD_DP32(val, TOY, MON, tm.tm_mon + 1);
        val = FIELD_DP32(val, TOY, DAY, tm.tm_mday);
        val = FIELD_DP32(val, TOY, HOUR, tm.tm_hour);
        val = FIELD_DP32(val, TOY, MIN, tm.tm_min);
        val = FIELD_DP32(val, TOY, SEC, tm.tm_sec);
        break;
    case SYS_TOYREAD1:
        qemu_get_timedate(&tm, s->offset_toy);
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
        val = ls7a_rtc_ticks() + s->offset_rtc;
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

static void toymatch_write(LS7ARtcState *s, struct tm *tm, uint64_t val, int num)
{
    int64_t alarm_offset, year_diff, expire_time;

    if (FIELD_EX32(s->cntrctl, RTC_CTRL, TOYEN) &&
        FIELD_EX32(s->cntrctl, RTC_CTRL, EO)) {
        s->toymatch[num] = val;
        s->toy_timer[num].flag = 1;
        qemu_get_timedate(tm, s->offset_toy);
        tm->tm_sec = FIELD_EX32(val, TOY_MATCH, SEC);
        tm->tm_min = FIELD_EX32(val, TOY_MATCH, MIN);
        tm->tm_hour = FIELD_EX32(val, TOY_MATCH, HOUR);
        tm->tm_mday = FIELD_EX32(val, TOY_MATCH, DAY);
        tm->tm_mon = FIELD_EX32(val, TOY_MATCH, MON) - 1;
        year_diff = FIELD_EX32(val, TOY_MATCH, MON);
        year_diff = year_diff - (tm->tm_year & 0x3f);
        tm->tm_year = tm->tm_year + year_diff;
        alarm_offset = qemu_timedate_diff(tm) - s->offset_toy;
        if ((alarm_offset < 0) && (alarm_offset > -5)) {
            alarm_offset = 0;
        }
        expire_time = qemu_clock_get_ms(rtc_clock);
        expire_time += ((alarm_offset * 1000) + 100);
        timer_mod(s->toy_timer[num].timer, expire_time);
    }
}

static void rtcmatch_write(LS7ARtcState *s, uint64_t val, int num)
{
    uint64_t expire_time, time_offset;
    uint64_t now = ls7a_rtc_ticks();
    time_offset = val - now - s->offset_rtc;
    /* change ticks to ms */
    time_offset /= (LS7A_RTC_FREQ / 1000);
    expire_time = (now * 1000 / LS7A_RTC_FREQ) + time_offset;
    if (FIELD_EX32(s->cntrctl, RTC_CTRL, RTCEN) &&
        FIELD_EX32(s->cntrctl, RTC_CTRL, EO)) {
        s->rtcmatch[num] = val;
        /* set timer flag bit */
        s->rtc_timer[num].flag = 1;
        timer_mod(s->rtc_timer[num].timer, expire_time);
    }
}

static void ls7a_start_toymatch(LS7ARtcState *s)
{
    int i;
    uint64_t expire_time, now;

    now = qemu_clock_get_ms(rtc_clock);
    for (i = 0; i < TIMER_NUMS; i++) {
        if (s->toy_timer[i].flag) {
            expire_time = s->toy_timer[i].enable_offset + now;
            timer_mod(s->toy_timer[i].timer, expire_time);
        }
    }
}

static void ls7a_stop_toymatch(LS7ARtcState *s)
{
    int i;
    uint64_t now;

    now = qemu_clock_get_ms(rtc_clock);
    for (i = 0; i < TIMER_NUMS; i++) {
        if (s->toy_timer[i].flag) {
            s->toy_timer[i].enable_offset = s->toy_timer[i].timer->expire_time
                                            - now;
            timer_del(s->toy_timer[i].timer);
        }
    }
}

static void ls7a_start_rtcmatch(LS7ARtcState *s)
{
    int i;
    uint64_t expire_time, now;

    now = ls7a_rtc_ticks();
    for (i = 0; i < TIMER_NUMS; i++) {
        if (s->rtc_timer[i].flag) {
            /* change rtc counters to ms */
            expire_time = now + s->rtc_timer[i].enable_offset;
            expire_time /= (LS7A_RTC_FREQ / 1000);
            timer_mod(s->rtc_timer[i].timer, expire_time);
        }
    }
}

static void ls7a_stop_rtcmatch(LS7ARtcState *s)
{
    int i;
    uint64_t now;

    now = ls7a_rtc_ticks();
    for (i = 0; i < TIMER_NUMS; i++) {
        if (s->rtc_timer[i].flag) {
            s->rtc_timer[i].enable_offset = s->rtcmatch[i] -
                                            now - s->offset_rtc;
            timer_del(s->rtc_timer[i].timer);
        }
    }
}

static void ls7a_rtc_write(void *opaque, hwaddr addr,
                           uint64_t val, unsigned size)
{
    int ctrl_diff, ctrl_old;
    LS7ARtcState *s = LS7A_RTC(opaque);
    struct tm tm;

    switch (addr) {
    case SYS_TOYWRITE0:
        qemu_get_timedate(&tm, s->offset_toy);
        tm.tm_sec = FIELD_EX32(val, TOY, SEC);
        tm.tm_min = FIELD_EX32(val, TOY, MIN);
        tm.tm_hour = FIELD_EX32(val, TOY, HOUR);
        tm.tm_mday = FIELD_EX32(val, TOY, DAY);
        tm.tm_mon = FIELD_EX32(val, TOY, MON) - 1;
        s->offset_toy = qemu_timedate_diff(&tm);
    break;
    case SYS_TOYWRITE1:
        qemu_get_timedate(&tm, s->offset_toy);
        tm.tm_year = val;
        s->offset_toy = qemu_timedate_diff(&tm);
        break;
    case SYS_TOYMATCH0:
        toymatch_write(s, &tm, val, 0);
        break;
    case SYS_TOYMATCH1:
        toymatch_write(s, &tm, val, 1);
        break;
    case SYS_TOYMATCH2:
        toymatch_write(s, &tm, val, 2);
        break;
    case SYS_RTCCTRL:
        ctrl_old = s->cntrctl;
        ctrl_diff = ctrl_old ^ val;
        s->cntrctl = val;
        /* if EO changed */
        if (FIELD_EX32(ctrl_diff, RTC_CTRL, EO)) {
            /* enable EO */
            if (FIELD_EX32(val, RTC_CTRL, EO)) {
                /* if toy or rtc enabled now */
                if (FIELD_EX32(val, RTC_CTRL, TOYEN)) {
                    ls7a_start_toymatch(s);
                }
                if (FIELD_EX32(val, RTC_CTRL, RTCEN)) {
                    ls7a_start_rtcmatch(s);
                }
            } else {
                /* if EO disabled */
                if (FIELD_EX32(ctrl_old, RTC_CTRL, TOYEN)) {
                    ls7a_stop_toymatch(s);
                }
                if (FIELD_EX32(ctrl_old, RTC_CTRL, RTCEN)) {
                    ls7a_stop_rtcmatch(s);
                }
            }
        } else {
            /* EO not changed */
            if (FIELD_EX32(ctrl_diff, RTC_CTRL, TOYEN)) {
                /* enable TOYEN */
                if (FIELD_EX32(val, RTC_CTRL, TOYEN)) {
                    if (FIELD_EX32(val, RTC_CTRL, EO)) {
                        ls7a_start_toymatch(s);
                    }
                } else {
                /* disable TOYEN */
                    if (FIELD_EX32(ctrl_old, RTC_CTRL, EO)) {
                        /* if EO not enabled, rtc has already stopeed */
                        ls7a_stop_toymatch(s);
                    }
                }
            }
            if (FIELD_EX32(ctrl_diff, RTC_CTRL, RTCEN)) {
                /* enable RTCEN */
                if (FIELD_EX32(val, RTC_CTRL, RTCEN)) {
                    if (FIELD_EX32(val, RTC_CTRL, EO)) {
                        ls7a_start_rtcmatch(s);
                    }
                } else {
                /* disable RTCEN */
                    if (FIELD_EX32(ctrl_old, RTC_CTRL, EO)) {
                        /* if EO not enabled, rtc has already stopeed */
                        ls7a_stop_rtcmatch(s);
                    }
                }
            }
        }
        break;
    case SYS_RTCWRTIE0:
        s->offset_rtc = val - ls7a_rtc_ticks();
        break;
    case SYS_RTCMATCH0:
        rtcmatch_write(s, val, 0);
        break;
    case SYS_RTCMATCH1:
        rtcmatch_write(s, val, 1);
        break;
    case SYS_RTCMATCH2:
        rtcmatch_write(s, val, 2);
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
    LS7ARtcTimer *timer = opaque;
    LS7ARtcState *s = timer->d;

    if (FIELD_EX32(s->cntrctl, RTC_CTRL, TOYEN)
        && FIELD_EX32(s->cntrctl, RTC_CTRL, EO)) {
        timer->flag = 0;
        qemu_irq_pulse(s->irq);
    }
}

static void rtc_timer(void *opaque)
{
    LS7ARtcTimer *timer = opaque;
    LS7ARtcState *s = timer->d;

    if (FIELD_EX32(s->cntrctl, RTC_CTRL, RTCEN)
        && FIELD_EX32(s->cntrctl, RTC_CTRL, EO)) {
        timer->flag = 0;
        qemu_irq_pulse(s->irq);
    }
}

static void ls7a_rtc_realize(DeviceState *dev, Error **errp)
{
    int i;
    SysBusDevice *sbd = SYS_BUS_DEVICE(dev);
    LS7ARtcState *d = LS7A_RTC(sbd);
    memory_region_init_io(&d->iomem, NULL, &ls7a_rtc_ops,
                         (void *)d, "ls7a_rtc", 0x100);

    sysbus_init_irq(sbd, &d->irq);

    sysbus_init_mmio(sbd, &d->iomem);
    for (i = 0; i < TIMER_NUMS; i++) {
        d->toy_timer[i].d = d;
        d->rtc_timer[i].d = d;
        d->toy_timer[i].timer = timer_new_ms(rtc_clock, toy_timer,
                                             &d->toy_timer[i]);
        d->rtc_timer[i].timer = timer_new_ms(rtc_clock, rtc_timer,
                                             &d->rtc_timer[i]);
        d->toy_timer[i].flag = 0;
        d->rtc_timer[i].flag = 0;
    }
    d->offset_toy = 0;
    d->offset_rtc = 0;

    create_unimplemented_device("mmio fallback 1", 0x10013ffc, 0x4);
}

static int ls7a_rtc_pre_save(void *opaque)
{
    LS7ARtcState *s = LS7A_RTC(opaque);
    struct tm tm;
    int64_t year_diff, toy_val, rtc_val, rtc_diff, now;
    int i;
    now = ls7a_rtc_ticks();
    qemu_get_timedate(&tm, s->offset_toy);
    for (i = 0; i < TIMER_NUMS; i++) {
        if (s->toy_timer[i].flag) {
            toy_val = s->toymatch[i];
            tm.tm_sec = FIELD_EX32(toy_val, TOY_MATCH, SEC);
            tm.tm_min = FIELD_EX32(toy_val, TOY_MATCH, MIN);
            tm.tm_hour = FIELD_EX32(toy_val, TOY_MATCH, HOUR);
            tm.tm_mday = FIELD_EX32(toy_val, TOY_MATCH, DAY);
            tm.tm_mon = FIELD_EX32(toy_val, TOY_MATCH, MON) - 1;
            year_diff = FIELD_EX32(toy_val, TOY_MATCH, MON);
            year_diff = year_diff - (tm.tm_year & 0x3f);
            tm.tm_year = tm.tm_year + year_diff;
            s->toy_timer[i].save_offset = qemu_timedate_diff(&tm)
                                          - s->offset_toy;
        }
        if (s->rtc_timer[i].flag) {
            rtc_val = s->rtcmatch[i];
            rtc_diff = rtc_val - now - s->offset_rtc;
            s->rtc_timer[i].save_offset = rtc_diff;
        }
    }
    return 0;
}

static int ls7a_rtc_post_load(void *opaque, int version_id)
{
    LS7ARtcState *s = LS7A_RTC(opaque);
    int64_t expire_time;
    int i;
    uint64_t now, ticks;

    now = qemu_clock_get_ms(rtc_clock);
    ticks = ls7a_rtc_ticks();
    for (i = 0; i < TIMER_NUMS; i++) {
        if (s->toy_timer[i].flag) {
            expire_time = now + (s->toy_timer[i].save_offset * 1000);
            timer_mod(s->toy_timer[i].timer, expire_time);
        }
        if (s->rtc_timer[i].flag) {
            expire_time = ticks + s->rtc_timer[i].save_offset;
            expire_time = (expire_time * 1000 / LS7A_RTC_FREQ);
            timer_mod(s->rtc_timer[i].timer, expire_time);
        }
    }
    return 0;
}

static const VMStateDescription vmstate_ls7a_rtc_timer = {
    .name = "ls7a_rtc_timer",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_INT32(flag, LS7ARtcTimer),
        VMSTATE_INT64(enable_offset, LS7ARtcTimer),
        VMSTATE_INT64(save_offset, LS7ARtcTimer),
        VMSTATE_END_OF_LIST()
    }
};

static const VMStateDescription vmstate_ls7a_rtc = {
    .name = "ls7a_rtc",
    .version_id = 1,
    .minimum_version_id = 1,
    .pre_save = ls7a_rtc_pre_save,
    .post_load = ls7a_rtc_post_load,
    .fields = (VMStateField[]) {
        VMSTATE_INT64(offset_toy, LS7ARtcState),
        VMSTATE_INT64(offset_rtc, LS7ARtcState),
        VMSTATE_UINT32_ARRAY(toymatch, LS7ARtcState, TIMER_NUMS),
        VMSTATE_UINT32_ARRAY(rtcmatch, LS7ARtcState, TIMER_NUMS),
        VMSTATE_UINT32(cntrctl, LS7ARtcState),
        VMSTATE_STRUCT_ARRAY(toy_timer, LS7ARtcState, TIMER_NUMS, 1,
                             vmstate_ls7a_rtc_timer, LS7ARtcTimer),
        VMSTATE_STRUCT_ARRAY(rtc_timer, LS7ARtcState, TIMER_NUMS, 1,
                             vmstate_ls7a_rtc_timer, LS7ARtcTimer),
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
    .instance_size = sizeof(LS7ARtcState),
    .class_init    = ls7a_rtc_class_init,
};

static void ls7a_rtc_register_types(void)
{
    type_register_static(&ls7a_rtc_info);
}

type_init(ls7a_rtc_register_types)
