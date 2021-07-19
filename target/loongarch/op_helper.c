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
#include "qemu/crc32c.h"
#include <zlib.h>

/* Exceptions helpers */
void helper_raise_exception_err(CPULoongArchState *env, uint32_t exception,
                                int error_code)
{
    do_raise_exception_err(env, exception, error_code, 0);
}

void helper_raise_exception(CPULoongArchState *env, uint32_t exception)
{
    do_raise_exception(env, exception, GETPC());
}

target_ulong helper_cto_w(CPULoongArchState *env, target_ulong rj)
{
    uint32_t v = (uint32_t)rj;
    int temp = 0;

    while ((v & 0x1) == 1) {
        temp++;
        v = v >> 1;
    }

    return (target_ulong)temp;
}

target_ulong helper_ctz_w(CPULoongArchState *env, target_ulong rj)
{
    uint32_t v = (uint32_t)rj;

    if (v == 0) {
        return 32;
    }

    int temp = 0;
    while ((v & 0x1) == 0) {
        temp++;
        v = v >> 1;
    }

    return (target_ulong)temp;
}

target_ulong helper_cto_d(CPULoongArchState *env, target_ulong rj)
{
    uint64_t v = rj;
    int temp = 0;

    while ((v & 0x1) == 1) {
        temp++;
        v = v >> 1;
    }

    return (target_ulong)temp;
}

target_ulong helper_ctz_d(CPULoongArchState *env, target_ulong rj)
{
    uint64_t v = rj;

    if (v == 0) {
        return 64;
    }

    int temp = 0;
    while ((v & 0x1) == 0) {
        temp++;
        v = v >> 1;
    }

    return (target_ulong)temp;
}

target_ulong helper_bitrev_w(CPULoongArchState *env, target_ulong rj)
{
    int32_t v = (int32_t)rj;
    const int SIZE = 32;
    uint8_t bytes[SIZE];

    int i;
    for (i = 0; i < SIZE; i++) {
        bytes[i] = v & 0x1;
        v = v >> 1;
    }
    /* v == 0 */
    for (i = 0; i < SIZE; i++) {
        v = v | ((uint32_t)bytes[i] << (SIZE - 1 - i));
    }

    return (target_ulong)(int32_t)v;
}

target_ulong helper_bitrev_d(CPULoongArchState *env, target_ulong rj)
{
    uint64_t v = rj;
    const int SIZE = 64;
    uint8_t bytes[SIZE];

    int i;
    for (i = 0; i < SIZE; i++) {
        bytes[i] = v & 0x1;
        v = v >> 1;
    }
    /* v == 0 */
    for (i = 0; i < SIZE; i++) {
        v = v | ((uint64_t)bytes[i] << (SIZE - 1 - i));
    }

    return (target_ulong)v;
}

static inline target_ulong bitswap(target_ulong v)
{
    v = ((v >> 1) & (target_ulong)0x5555555555555555ULL) |
        ((v & (target_ulong)0x5555555555555555ULL) << 1);
    v = ((v >> 2) & (target_ulong)0x3333333333333333ULL) |
        ((v & (target_ulong)0x3333333333333333ULL) << 2);
    v = ((v >> 4) & (target_ulong)0x0F0F0F0F0F0F0F0FULL) |
        ((v & (target_ulong)0x0F0F0F0F0F0F0F0FULL) << 4);
    return v;
}

target_ulong helper_loongarch_dbitswap(target_ulong rj)
{
    return bitswap(rj);
}

target_ulong helper_loongarch_bitswap(target_ulong rt)
{
    return (int32_t)bitswap(rt);
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

target_ulong helper_crc32(target_ulong val, target_ulong m, uint32_t sz)
{
    uint8_t buf[8];
    target_ulong mask = ((sz * 8) == 64) ? -1ULL : ((1ULL << (sz * 8)) - 1);

    m &= mask;
    stq_le_p(buf, m);
    return (int32_t) (crc32(val ^ 0xffffffff, buf, sz) ^ 0xffffffff);
}

target_ulong helper_crc32c(target_ulong val, target_ulong m, uint32_t sz)
{
    uint8_t buf[8];
    target_ulong mask = ((sz * 8) == 64) ? -1ULL : ((1ULL << (sz * 8)) - 1);
    m &= mask;
    stq_le_p(buf, m);
    return (int32_t) (crc32c(val, buf, sz) ^ 0xffffffff);
}

target_ulong helper_cpucfg(CPULoongArchState *env, target_ulong rj)
{
    target_ulong r = 0;

    switch (rj) {
    case 0:
        r = env->CSR_MCSR0 & 0xffffffff;
        break;
    case 1:
        r = (env->CSR_MCSR0 & 0xffffffff00000000) >> 32;
        break;
    case 2:
        r = env->CSR_MCSR1 & 0xffffffff;
        break;
    case 3:
        r = (env->CSR_MCSR1 & 0xffffffff00000000) >> 32;
        break;
    case 4:
        r = env->CSR_MCSR2 & 0xffffffff;
        break;
    case 5:
        r = (env->CSR_MCSR2 & 0xffffffff00000000) >> 32;
        break;
    case 6:
        r = env->CSR_MCSR3 & 0xffffffff;
        break;
    case 10:
        r = env->CSR_MCSR8 & 0xffffffff;
        break;
    case 11:
        r = (env->CSR_MCSR8 & 0xffffffff00000000) >> 32;
        break;
    case 12:
        r = env->CSR_MCSR9 & 0xffffffff;
        break;
    case 13:
        r = (env->CSR_MCSR9 & 0xffffffff00000000) >> 32;
        break;
    case 14:
        r = env->CSR_MCSR10 & 0xffffffff;
        break;
    case 30:
        r = env->CSR_MCSR24 & 0xffffffff;
        break;
    }
    return r;
}
