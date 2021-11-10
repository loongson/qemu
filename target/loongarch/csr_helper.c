/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * LoongArch emulation helpers for csr registers
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 */

#include "qemu/osdep.h"
#include "qemu/main-loop.h"
#include "cpu.h"
#include "internals.h"
#include "qemu/host-utils.h"
#include "exec/helper-proto.h"
#include "exec/exec-all.h"
#include "exec/cpu_ldst.h"
#include "hw/irq.h"
#include "cpu-csr.h"
#include "hw/loongarch/loongarch.h"
#include "tcg/tcg-ldst.h"

target_ulong helper_csr_rdq(CPULoongArchState *env, uint64_t csr)
{
    LoongArchCPU *cpu;
    int64_t v;

    switch (csr) {
    case LOONGARCH_CSR_PGD:
        if (env->CSR_TLBRERA & 0x1) {
            v = env->CSR_TLBRBADV;
        } else {
            v = env->CSR_BADV;
        }

        if ((v >> 63) & 0x1) {
            v = env->CSR_PGDH;
        } else {
            v = env->CSR_PGDL;
        }
        break;
    case LOONGARCH_CSR_CPUID:
        v = (env_cpu(env))->cpu_index;
        break;
    case LOONGARCH_CSR_TVAL:
        cpu = LOONGARCH_CPU(env_cpu(env));
        v = cpu_loongarch_get_stable_timer_ticks(cpu);
        break;
    default:
        break;
    }

    return v;
}

target_ulong helper_csr_wrq(CPULoongArchState *env, target_ulong val,
                            uint64_t csr)
{
    LoongArchCPU *cpu;
    int64_t old_v = -1;

    switch (csr) {
    case LOONGARCH_CSR_ESTAT:
        /* Only IS[1:0] can be write */
        env->CSR_ESTAT = FIELD_DP64(env->CSR_ESTAT, CSR_ESTAT, IS, val & 0x3);
        break;
    case LOONGARCH_CSR_ASID:
        old_v = env->CSR_ASID;
        /* Only ASID filed of CSR_ASID can be write. */
        env->CSR_ASID = FIELD_DP64(env->CSR_ASID, CSR_ASID, ASID,
                                   val & R_CSR_ASID_ASID_MASK);
        if (old_v != val) {
            tlb_flush(env_cpu(env));
        }
        break;
    case LOONGARCH_CSR_TCFG:
        cpu = LOONGARCH_CPU(env_cpu(env));
        old_v = env->CSR_TCFG;
        cpu_loongarch_store_stable_timer_config(cpu, val);
        break;
    case LOONGARCH_CSR_TICLR:
        old_v = 0;
        env->CSR_ESTAT &= ~(1 << IRQ_TIMER);
        cpu_reset_interrupt(env_cpu(env), CPU_INTERRUPT_HARD);
        break;
    default:
        break;
    }

    return old_v;
}

target_ulong helper_csr_xchgq(CPULoongArchState *env, target_ulong new_val,
                              target_ulong mask, uint64_t csr_num)
{
    unsigned csr_offset = cpu_csr_offset(csr_num);
    if (csr_offset == 0) {
        /* CSR is undefined: read as 0, write ignored. */
        return 0;
    }

    uint64_t *csr = (void *)env + csr_offset;
    uint64_t old_val = *csr;

    new_val = (new_val & mask) | (old_val & ~mask);

    if (csr_num == LOONGARCH_CSR_TCFG) {
        LoongArchCPU *cpu = LOONGARCH_CPU(env_cpu(env));
        cpu_loongarch_store_stable_timer_config(cpu, new_val);
    } else {
        *csr = new_val;
    }
    return old_val;
}
