/*
 * LoongArch emulation helpers for qemu.
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "qemu/main-loop.h"
#include "cpu.h"
#include "qemu/host-utils.h"
#include "exec/helper-proto.h"
#include "exec/exec-all.h"
#include "exec/cpu_ldst.h"
#include "internals.h"
#include "qemu/crc32c.h"
#include <zlib.h>

/* Exceptions helpers */
void helper_raise_exception(CPULoongArchState *env, uint32_t exception)
{
    do_raise_exception(env, exception, GETPC());
}


target_ulong helper_bitrev_w(target_ulong rj)
{
    return (int32_t)revbit32(rj);
}

target_ulong helper_bitrev_d(target_ulong rj)
{
    return revbit64(rj);
}

target_ulong helper_bitswap(target_ulong v)
{
    v = ((v >> 1) & (target_ulong)0x5555555555555555ULL) |
        ((v & (target_ulong)0x5555555555555555ULL) << 1);
    v = ((v >> 2) & (target_ulong)0x3333333333333333ULL) |
        ((v & (target_ulong)0x3333333333333333ULL) << 2);
    v = ((v >> 4) & (target_ulong)0x0F0F0F0F0F0F0F0FULL) |
        ((v & (target_ulong)0x0F0F0F0F0F0F0F0FULL) << 4);
    return v;
}

/* loongarch assert op */
void helper_asrtle_d(CPULoongArchState *env, target_ulong rj, target_ulong rk)
{
    if (rj > rk) {
        do_raise_exception(env, EXCP_ADE, GETPC());
    }
}

void helper_asrtgt_d(CPULoongArchState *env, target_ulong rj, target_ulong rk)
{
    if (rj <= rk) {
        do_raise_exception(env, EXCP_ADE, GETPC());
    }
}

target_ulong helper_crc32(target_ulong val, target_ulong m, uint64_t sz)
{
    uint8_t buf[8];
    target_ulong mask = ((sz * 8) == 64) ? -1ULL : ((1ULL << (sz * 8)) - 1);

    m &= mask;
    stq_le_p(buf, m);
    return (int32_t) (crc32(val ^ 0xffffffff, buf, sz) ^ 0xffffffff);
}

target_ulong helper_crc32c(target_ulong val, target_ulong m, uint64_t sz)
{
    uint8_t buf[8];
    target_ulong mask = ((sz * 8) == 64) ? -1ULL : ((1ULL << (sz * 8)) - 1);
    m &= mask;
    stq_le_p(buf, m);
    return (int32_t) (crc32c(val, buf, sz) ^ 0xffffffff);
}

target_ulong helper_cpucfg(CPULoongArchState *env, target_ulong rj)
{
    return env->cpucfg[rj];
}

#ifndef CONFIG_USER_ONLY
void helper_ertn(CPULoongArchState *env)
{
    if (env->CSR_TLBRERA & 0x1) {
        env->CSR_CRMD = FIELD_DP64(env->CSR_CRMD, CSR_CRMD, PLV,
                                   FIELD_EX64(env->CSR_TLBRPRMD, CSR_TLBRPRMD, PPLV));
        env->CSR_CRMD = FIELD_DP64(env->CSR_CRMD, CSR_CRMD, IE,
                                   FIELD_EX64(env->CSR_TLBRPRMD, CSR_TLBRPRMD, PIE));
        /* Clear Refill flag and set pc */
        env->CSR_TLBRERA &= (~0x1);
        env->pc = env->CSR_TLBRERA;
        if (qemu_loglevel_mask(CPU_LOG_INT)) {
            qemu_log("%s: TLBRERA 0x%lx\n", __func__, env->CSR_TLBRERA);
        }
    } else {
        env->CSR_CRMD = FIELD_DP64(env->CSR_CRMD, CSR_CRMD, PLV,
                                   FIELD_EX64(env->CSR_PRMD, CSR_PRMD, PPLV));
        env->CSR_CRMD = FIELD_DP64(env->CSR_CRMD, CSR_CRMD, IE,
                                   FIELD_EX64(env->CSR_PRMD, CSR_PRMD, PIE));
        /* set pc*/
        env->pc = env->CSR_ERA;
        if (qemu_loglevel_mask(CPU_LOG_INT)) {
            qemu_log("%s: ERA 0x%lx\n", __func__, env->CSR_ERA);
        }
    }

    env->lladdr = 1;
}

void helper_idle(CPULoongArchState *env)
{
    CPUState *cs = env_cpu(env);

    cs->halted = 1;
    cpu_reset_interrupt(cs, CPU_INTERRUPT_WAKE);
    /*
     * Last instruction in the block, PC was updated before
     * - no need to recover PC and icount
     */
    do_raise_exception(env, EXCP_HLT, 0);
}

uint64_t helper_rdtime_d(CPULoongArchState *env)
{
     return cpu_loongarch_get_stable_counter(env);
}

#endif /* !CONFIG_USER_ONLY */
