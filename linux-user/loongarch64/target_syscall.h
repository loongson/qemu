/*
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef LOONGARCH_TARGET_SYSCALL_H
#define LOONGARCH_TARGET_SYSCALL_H

/*
 * this struct defines the way the registers are stored on the
 * stack during a system call.
 */

struct target_pt_regs {
    /* Saved main processor registers. */
    target_ulong regs[32];

    /* Saved special registers. */
    target_ulong csr_crmd;
    target_ulong csr_prmd;
    target_ulong csr_euen;
    target_ulong csr_ecfg;
    target_ulong csr_estat;
    target_ulong csr_era;
    target_ulong csr_badvaddr;
    target_ulong orig_a0;
    target_ulong __last[0];
};

#define UNAME_MACHINE "loongarch"
#define UNAME_MINIMUM_RELEASE "4.19.0"

#define TARGET_MINSIGSTKSZ 2048
#define TARGET_MCL_CURRENT 1
#define TARGET_MCL_FUTURE  2
#define TARGET_MCL_ONFAULT 4

#define TARGET_FORCE_SHMLBA

static inline abi_ulong target_shmlba(CPULoongArchState *env)
{
    return 0x40000;
}

#endif
