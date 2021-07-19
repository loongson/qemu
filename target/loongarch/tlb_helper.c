/*
 * LoongArch tlb emulation helpers for qemu.
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "cpu.h"
#include "cpu-csr.h"
#include "exec/helper-proto.h"
#include "exec/exec-all.h"
#include "exec/cpu_ldst.h"
#include "exec/log.h"

enum {
    TLBRET_PE = -7,
    TLBRET_XI = -6,
    TLBRET_RI = -5,
    TLBRET_DIRTY = -4,
    TLBRET_INVALID = -3,
    TLBRET_NOMATCH = -2,
    TLBRET_BADADDR = -1,
    TLBRET_MATCH = 0
};

static void raise_mmu_exception(CPULoongArchState *env, target_ulong address,
                                MMUAccessType access_type, int tlb_error)
{
    CPUState *cs = env_cpu(env);
    int exception = 0, error_code = 0;

    if (access_type == MMU_INST_FETCH) {
        error_code |= INST_INAVAIL;
    }

    switch (tlb_error) {
    default:
    case TLBRET_BADADDR:
        exception = EXCP_ADE;
        break;
    case TLBRET_NOMATCH:
        /* No TLB match for a mapped address */
        if (access_type == MMU_DATA_STORE) {
            exception = EXCP_TLBS;
        } else {
            exception = EXCP_TLBL;
        }
        error_code |= TLB_NOMATCH;
        break;
    case TLBRET_INVALID:
        /* TLB match with no valid bit */
        if (access_type == MMU_DATA_STORE) {
            exception = EXCP_TLBS;
        } else {
            exception = EXCP_TLBL;
        }
        break;
    case TLBRET_DIRTY:
        exception = EXCP_TLBM;
        break;
    case TLBRET_XI:
        /* Execute-Inhibit Exception */
        exception = EXCP_TLBXI;
        break;
    case TLBRET_RI:
        /* Read-Inhibit Exception */
        exception = EXCP_TLBRI;
        break;
    case TLBRET_PE:
        /* Privileged Exception */
        exception = EXCP_TLBPE;
        break;
    }

    if (tlb_error == TLBRET_NOMATCH) {
        env->CSR_TLBRBADV = address;
        env->CSR_TLBREHI = address & (TARGET_PAGE_MASK << 1);
        cs->exception_index = exception;
        env->error_code = error_code;
        return;
    }

    /* Raise exception */
    env->CSR_BADV = address;
    cs->exception_index = exception;
    env->error_code = error_code;
    env->CSR_TLBEHI = address & (TARGET_PAGE_MASK << 1);
}

bool loongarch_cpu_tlb_fill(CPUState *cs, vaddr address, int size,
                       MMUAccessType access_type, int mmu_idx,
                       bool probe, uintptr_t retaddr)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;
    int ret = TLBRET_BADADDR;

    /* data access */
    raise_mmu_exception(env, address, access_type, ret);
    do_raise_exception_err(env, cs->exception_index, env->error_code, retaddr);
}
