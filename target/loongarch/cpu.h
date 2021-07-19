/*
 * QEMU LoongArch CPU
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef LOONGARCH_CPU_H
#define LOONGARCH_CPU_H

#include "exec/cpu-defs.h"
#include "fpu/softfloat-types.h"
#include "hw/clock.h"
#include "cpu-qom.h"
#include "cpu-csr.h"

#define ISA_LA32       0x00000001ULL
#define ISA_LA64       0x00000002ULL
#define INSN_LOONGARCH 0x00010000ULL

#define CPU_LA32       (ISA_LA32)
#define CPU_LA64       (ISA_LA32 | ISA_LA64)

#define TCG_GUEST_DEFAULT_MO (0)
#define UNASSIGNED_CPU_ID 0xFFFFFFFF

typedef union fpr_t fpr_t;
union fpr_t {
    float64  fd;   /* ieee double precision */
    float32  fs[2];/* ieee single precision */
    uint64_t d;    /* binary double fixed-point */
    uint32_t w[2]; /* binary single fixed-point */
};

/*
 * define FP_ENDIAN_IDX to access the same location
 * in the fpr_t union regardless of the host endianness
 */
#if defined(HOST_WORDS_BIGENDIAN)
#  define FP_ENDIAN_IDX 1
#else
#  define FP_ENDIAN_IDX 0
#endif

typedef struct CPULoongArchFPUContext CPULoongArchFPUContext;
struct CPULoongArchFPUContext {
    /* Floating point registers */
    fpr_t fpr[32];
    float_status fp_status;

    bool cf[8];
    /*
     * fcsr0
     * 31:29 |28:24 |23:21 |20:16 |15:10 |9:8 |7  |6  |5 |4:0
     *        Cause         Flags         RM   DAE TM     Enables
     */
    uint32_t fcsr0;
    uint32_t fcsr0_mask;
    uint32_t vcsr16;

#define FCSR0_M1    0xdf         /* FCSR1 mask, DAE, TM and Enables */
#define FCSR0_M2    0x1f1f0000   /* FCSR2 mask, Cause and Flags */
#define FCSR0_M3    0x300        /* FCSR3 mask, Round Mode */
#define FCSR0_RM    8            /* Round Mode bit num on fcsr0 */
#define GET_FP_CAUSE(reg)        (((reg) >> 24) & 0x1f)
#define GET_FP_ENABLE(reg)       (((reg) >>  0) & 0x1f)
#define GET_FP_FLAGS(reg)        (((reg) >> 16) & 0x1f)
#define SET_FP_CAUSE(reg, v)      do { (reg) = ((reg) & ~(0x1f << 24)) | \
                                               ((v & 0x1f) << 24);       \
                                     } while (0)
#define SET_FP_ENABLE(reg, v)     do { (reg) = ((reg) & ~(0x1f <<  0)) | \
                                               ((v & 0x1f) << 0);        \
                                     } while (0)
#define SET_FP_FLAGS(reg, v)      do { (reg) = ((reg) & ~(0x1f << 16)) | \
                                               ((v & 0x1f) << 16);       \
                                     } while (0)
#define UPDATE_FP_FLAGS(reg, v)   do { (reg) |= ((v & 0x1f) << 16); } while (0)
#define FP_INEXACT        1
#define FP_UNDERFLOW      2
#define FP_OVERFLOW       4
#define FP_DIV0           8
#define FP_INVALID        16
};

#define TARGET_INSN_START_EXTRA_WORDS 2
#define LOONGARCH_FPU_MAX 1
#define N_IRQS      14

enum loongarch_feature {
    LA_FEATURE_3A5000,
};

typedef struct TCState TCState;
struct TCState {
    target_ulong gpr[32];
    target_ulong PC;
};

typedef struct CPULoongArchState CPULoongArchState;
struct CPULoongArchState {
    TCState active_tc;
    CPULoongArchFPUContext active_fpu;

    uint32_t current_tc;
    uint64_t scr[4];
    uint32_t current_fpu;

    /* LoongArch CSR register */
    CPU_LOONGARCH_CSR
    target_ulong lladdr; /* LL virtual address compared against SC */
    target_ulong llval;

    CPULoongArchFPUContext fpus[LOONGARCH_FPU_MAX];

    /* QEMU */
    int error_code;
    uint32_t hflags;    /* CPU State */
#define TLB_NOMATCH   0x1
#define INST_INAVAIL  0x2 /* Invalid instruction word for BadInstr */
    /* TMASK defines different execution modes */
#define LOONGARCH_HFLAG_TMASK  0x1F5807FF
#define LOONGARCH_HFLAG_KU     0x00003 /* kernel/supervisor/user mode mask   */
#define LOONGARCH_HFLAG_UM     0x00003 /* user mode flag                     */
#define LOONGARCH_HFLAG_KM     0x00000 /* kernel mode flag                   */
#define LOONGARCH_HFLAG_64     0x00008 /* 64-bit instructions enabled        */
#define LOONGARCH_HFLAG_FPU    0x00020 /* FPU enabled                        */
#define LOONGARCH_HFLAG_F64    0x00040 /* 64-bit FPU enabled                 */
#define LOONGARCH_HFLAG_BMASK  0x3800
#define LOONGARCH_HFLAG_B      0x00800 /* Unconditional branch               */
#define LOONGARCH_HFLAG_BC     0x01000 /* Conditional branch                 */
#define LOONGARCH_HFLAG_BR     0x02000 /* branch to register (can't link TB) */
#define LOONGARCH_HFLAG_FRE   0x2000000 /* FRE enabled */
#define LOONGARCH_HFLAG_ELPA  0x4000000
    target_ulong btarget;        /* Jump / branch target               */
    target_ulong bcond;          /* Branch condition (if needed)       */

    /* Fields up to this point are cleared by a CPU reset */
    struct {} end_reset_fields;

    /* Fields after this point are preserved across CPU reset. */
    uint64_t features;
    void *irq[N_IRQS];
    QEMUTimer *timer; /* Internal timer */
    target_ulong exception_base; /* ExceptionBase input to the core */
};

/**
 * LoongArchCPU:
 * @env: #CPULoongArchState
 * @clock: this CPU input clock (may be connected
 *         to an output clock from another device).
 *
 * A LoongArch CPU.
 */
struct LoongArchCPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    Clock *clock;
    CPUNegativeOffsetState neg;
    CPULoongArchState env;
    uint32_t id;
    int32_t node_id; /* NUMA node this CPU belongs to */
    int32_t core_id;
};

target_ulong exception_resume_pc(CPULoongArchState *env);

static inline void cpu_get_tb_cpu_state(CPULoongArchState *env,
                                        target_ulong *pc,
                                        target_ulong *cs_base,
                                        uint32_t *flags)
{
    *pc = env->active_tc.PC;
    *cs_base = 0;
    *flags = env->hflags & (LOONGARCH_HFLAG_TMASK | LOONGARCH_HFLAG_BMASK);
}

static inline LoongArchCPU *loongarch_env_get_cpu(CPULoongArchState *env)
{
    return container_of(env, LoongArchCPU, env);
}

#define ENV_GET_CPU(e) CPU(loongarch_env_get_cpu(e))

void loongarch_cpu_list(void);

#define CPU_INTERRUPT_WAKE CPU_INTERRUPT_TGT_INT_0

#define cpu_signal_handler cpu_loongarch_signal_handler
#define cpu_list loongarch_cpu_list

/* MMU modes definitions */
#define MMU_MODE0_SUFFIX _kernel
#define MMU_MODE1_SUFFIX _super
#define MMU_MODE2_SUFFIX _user
#define MMU_MODE3_SUFFIX _error
#define MMU_USER_IDX 2

static inline int cpu_mmu_index(CPULoongArchState *env, bool ifetch)
{
    return MMU_USER_IDX;
}

typedef CPULoongArchState CPUArchState;
typedef LoongArchCPU ArchCPU;

#include "exec/cpu-all.h"

/* Exceptions */
enum {
    EXCP_NONE          = -1,
    EXCP_INTE          = 0,
    EXCP_ADE,
    EXCP_SYSCALL,
    EXCP_BREAK,
    EXCP_FPDIS,
    EXCP_INE,
    EXCP_TRAP,
    EXCP_FPE,
    EXCP_TLBM,
    EXCP_TLBL,
    EXCP_TLBS,
    EXCP_TLBPE,
    EXCP_TLBXI,
    EXCP_TLBRI,

    EXCP_LAST = EXCP_TLBRI,
};

int cpu_loongarch_signal_handler(int host_signum, void *pinfo, void *puc);

#define LOONGARCH_CPU_TYPE_SUFFIX "-" TYPE_LOONGARCH_CPU
#define LOONGARCH_CPU_TYPE_NAME(model) model LOONGARCH_CPU_TYPE_SUFFIX
#define CPU_RESOLVING_TYPE TYPE_LOONGARCH_CPU

#include "exec/memattrs.h"

void loongarch_tcg_init(void);

void loongarch_cpu_dump_state(CPUState *cpu, FILE *f, int flags);

void QEMU_NORETURN do_raise_exception_err(CPULoongArchState *env,
                                          uint32_t exception,
                                          int error_code,
                                          uintptr_t pc);

static inline void QEMU_NORETURN do_raise_exception(CPULoongArchState *env,
                                                    uint32_t exception,
                                                    uintptr_t pc)
{
    do_raise_exception_err(env, exception, 0, pc);
}

static inline void compute_hflags(CPULoongArchState *env)
{
    env->hflags &= ~(LOONGARCH_HFLAG_64 | LOONGARCH_HFLAG_FPU |
                     LOONGARCH_HFLAG_KU | LOONGARCH_HFLAG_ELPA);

    env->hflags |= (env->CSR_CRMD & CSR_CRMD_PLV);
    env->hflags |= LOONGARCH_HFLAG_64;

    if (env->CSR_EUEN & CSR_EUEN_FPEN) {
        env->hflags |= LOONGARCH_HFLAG_FPU;
    }
}

const char *loongarch_exception_name(int32_t exception);

#endif /* LOONGARCH_CPU_H */
