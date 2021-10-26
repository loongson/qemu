/*
 * QEMU LoongArch timer support
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "hw/irq.h"
#include "hw/loongarch/loongarch.h"
#include "qemu/timer.h"
#include "cpu.h"

#define TIMER_PERIOD                10 /* 10 ns period for 100 Mhz frequency */
#define STABLETIMER_TICK_MASK       0xfffffffffffcUL
#define STABLETIMER_ENABLE          0x1UL

/* LoongArch timer */
uint64_t cpu_loongarch_get_stable_counter(CPULoongArchState *env)
{
    return qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) / TIMER_PERIOD;
}

uint64_t cpu_loongarch_get_stable_timer_ticks(CPULoongArchState *env)
{
    uint64_t now, expire;

    now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
    expire = timer_expire_time_ns(env->timer);

    return (expire - now) / TIMER_PERIOD;
}

void cpu_loongarch_store_stable_timer_config(CPULoongArchState *env,
                                             uint64_t value)
{
    uint64_t now, next;

    env->CSR_TCFG = value;
    if (value & STABLETIMER_ENABLE) {
        now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
        next = now + (value & STABLETIMER_TICK_MASK) * TIMER_PERIOD;
        timer_mod(env->timer, next);
    }
}

static void loongarch_stable_timer_cb(void *opaque)
{
    CPULoongArchState *env;
    uint64_t now, next;

    env = opaque;
    if (FIELD_EX64(env->CSR_TCFG, CSR_TCFG, PERIODIC)) {
        now = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL);
        next = now + (env->CSR_TCFG & STABLETIMER_TICK_MASK) * TIMER_PERIOD;
        timer_mod(env->timer, next);
    } else {
        env->CSR_TCFG = FIELD_DP64(env->CSR_TCFG, CSR_TCFG, EN, 0);
    }

   qemu_irq_raise(env->irq[IRQ_TIMER]);
}

void cpu_loongarch_clock_init(LoongArchCPU *cpu)
{
    CPULoongArchState *env = &cpu->env;

    env->timer = timer_new_ns(QEMU_CLOCK_VIRTUAL,
                              &loongarch_stable_timer_cb, env);
}
