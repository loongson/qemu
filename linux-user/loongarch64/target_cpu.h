/*
 * LoongArch specific CPU ABI and functions for linux-user
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef LOONGARCH_TARGET_CPU_H
#define LOONGARCH_TARGET_CPU_H

static inline void cpu_clone_regs_child(CPULoongArchState *env,
                                        target_ulong newsp, unsigned flags)
{
    if (newsp) {
        env->active_tc.gpr[3] = newsp;
    }
    env->active_tc.gpr[7] = 0;
    env->active_tc.gpr[4] = 0;
}

static inline void cpu_clone_regs_parent(CPULoongArchState *env,
                                         unsigned flags)
{
}

static inline void cpu_set_tls(CPULoongArchState *env, target_ulong newtls)
{
    env->active_tc.gpr[2] = newtls;
}

static inline abi_ulong get_sp_from_cpustate(CPULoongArchState *state)
{
    return state->active_tc.gpr[3];
}
#endif
