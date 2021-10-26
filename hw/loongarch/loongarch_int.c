/*
 * QEMU LOONGARCH interrupt support
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "qemu/main-loop.h"
#include "hw/irq.h"
#include "hw/loongarch/loongarch.h"
#include "cpu.h"

static void cpu_loongarch_irq_request(void *opaque, int irq, int level)
{
    LoongArchCPU *cpu = opaque;
    CPULoongArchState *env = &cpu->env;
    CPUState *cs = CPU(cpu);
    bool locked = false;

    if (irq < 0 || irq > N_IRQS) {
        return;
    }

    /* Make sure locking works even if BQL is already held by the caller */
    if (!qemu_mutex_iothread_locked()) {
        locked = true;
        qemu_mutex_lock_iothread();
    }

    if (level) {
        env->CSR_ESTAT |= 1 << irq;
    } else {
        env->CSR_ESTAT &= ~(1 << irq);
    }

    if (FIELD_EX64(env->CSR_ESTAT, CSR_ESTAT, IS)) {
        cpu_interrupt(cs, CPU_INTERRUPT_HARD);
    } else {
        cpu_reset_interrupt(cs, CPU_INTERRUPT_HARD);
    }

    if (locked) {
        qemu_mutex_unlock_iothread();
    }
}

void cpu_loongarch_init_irq(LoongArchCPU *cpu)
{
    CPULoongArchState *env = &cpu->env;
    qemu_irq *qi;
    int i;

    qi = qemu_allocate_irqs(cpu_loongarch_irq_request, cpu, N_IRQS);
    for (i = 0; i < N_IRQS; i++) {
        env->irq[i] = qi[i];
    }
    g_free(qi);
}


