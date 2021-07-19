/*
 * QEMU LoongArch CPU
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "fpu/softfloat-helpers.h"
#include "cpu.h"

extern const FloatRoundMode ieee_rm[4];

uint32_t fp_class_s(uint32_t arg, float_status *fst);
uint64_t fp_class_d(uint64_t arg, float_status *fst);

int ieee_ex_to_loongarch(int xcpt);

static inline void restore_rounding_mode(CPULoongArchState *env)
{
    set_float_rounding_mode(ieee_rm[(env->active_fpu.fcsr0 >> FCSR0_RM) & 0x3],
                            &env->active_fpu.fp_status);
}

static inline void restore_flush_mode(CPULoongArchState *env)
{
    set_flush_to_zero(0, &env->active_fpu.fp_status);
}

static inline void restore_fp_status(CPULoongArchState *env)
{
    restore_rounding_mode(env);
    restore_flush_mode(env);
}
