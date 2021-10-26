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
#include "hw/registerfields.h"
#include "cpu-csr.h"

#define TCG_GUEST_DEFAULT_MO (0)

#define UNASSIGNED_CPU_ID 0xFFFFFFFF

#define FCSR0_M1    0x1f         /* FCSR1 mask, Enables */
#define FCSR0_M2    0x1f1f0000   /* FCSR2 mask, Cause and Flags */
#define FCSR0_M3    0x300        /* FCSR3 mask, Round Mode */
#define FCSR0_RM    8            /* Round Mode bit num on fcsr0 */

FIELD(FCSR0, ENABLES, 0, 5)
FIELD(FCSR0, RM, 8, 2)
FIELD(FCSR0, FLAGS, 16, 5)
FIELD(FCSR0, CAUSE, 24, 5)

#define GET_FP_CAUSE(REG)      FIELD_EX32(REG, FCSR0, CAUSE)
#define SET_FP_CAUSE(REG, V)   FIELD_DP32(REG, FCSR0, CAUSE, V)
#define GET_FP_ENABLES(REG)    FIELD_EX32(REG, FCSR0, ENABLES)
#define SET_FP_ENABLES(REG, V) FIELD_DP32(REG, FCSR0, ENABLES, V)
#define GET_FP_FLAGS(REG)      FIELD_EX32(REG, FCSR0, FLAGS)
#define SET_FP_FLAGS(REG, V)   FIELD_DP32(REG, FCSR0, FLAGS, V)
#define UPDATE_FP_FLAGS(REG, V) \
    do { \
        (REG) |= FIELD_DP32(0, FCSR0, FLAGS, V); \
    } while (0)

#define FP_INEXACT        1
#define FP_UNDERFLOW      2
#define FP_OVERFLOW       4
#define FP_DIV0           8
#define FP_INVALID        16

extern const char * const regnames[];
extern const char * const fregnames[];

#define N_IRQS      14
#define IRQ_TIMER   11
#define IRQ_IPI     12

#define LOONGARCH_HFLAG_MODE   0x00003 /* LoongArch Mode*/
#define LOONGARCH_HFLAG_UM     0x00003 /* user mode flag                     */
#define LOONGARCH_HFLAG_KM     0x00000 /* kernel mode flag                   */

#define LOONGARCH_TLB_MAX 2112 /* 2048 STLB + 64 MTLB */

struct loongarch_tlb {
    target_ulong VPN;
    uint64_t PageMask;
    uint32_t PageSize;
    uint16_t ASID;
    uint64_t V0;     /* CSR_TLBLO[0] */
    uint64_t V1;

    uint64_t D0;     /* CSR_TLBLO[1] */
    uint64_t D1;

    uint64_t PLV0;   /* CSR_TLBLO[3:2] */
    uint64_t PLV1;

    uint64_t MAT0;   /* CSR_TLBLO[5:4] */
    uint64_t MAT1;

    uint64_t G;      /* CSR_TLBLO[6] */

    uint64_t PPN0;   /* CSR_TLBLO[47:12] */
    uint64_t PPN1;

    uint64_t NR0;    /* CSR_TLBLO[61] */
    uint64_t NR1;

    uint64_t NX0;    /* CSR_TLBLO[62] */
    uint64_t NX1;

    uint64_t NE;     /* CSR_TLBIDX[31] */

    uint64_t RPLV0;
    uint64_t RPLV1;  /* CSR_TLBLO[63] */
};
typedef struct loongarch_tlb loongarch_tlb;

typedef struct CPULoongArchState CPULoongArchState;
struct CPULoongArchState {
    uint64_t gpr[32];
    uint64_t pc;

    uint64_t fpr[32];
    float_status fp_status;
    bool cf[8];

    /*
     * fcsr0
     * 31:29 |28:24 |23:21 |20:16 |15:10 |9:8 |7:5 |4:0
     *        Cause         Flags         RM        Enables
     */
    uint32_t fcsr0;
    uint32_t fcsr0_mask;

    uint32_t cpucfg[49];

    uint64_t lladdr; /* LL virtual address compared against SC */
    uint64_t llval;

    uint64_t badaddr;

    /* LoongArch CSR registers */
    uint64_t CSR_CRMD;
    uint64_t CSR_PRMD;
    uint64_t CSR_EUEN;
    uint64_t CSR_MISC;
    uint64_t CSR_ECFG;
    uint64_t CSR_ESTAT;
    uint64_t CSR_ERA;
    uint64_t CSR_BADV;
    uint64_t CSR_BADI;
    uint64_t CSR_EENTRY;
    uint64_t CSR_TLBIDX;
    uint64_t CSR_TLBEHI;
    uint64_t CSR_TLBELO0;
    uint64_t CSR_TLBELO1;
    uint64_t CSR_ASID;
    uint64_t CSR_PGDL;
    uint64_t CSR_PGDH;
    uint64_t CSR_PGD;
    uint64_t CSR_PWCL;
    uint64_t CSR_PWCH;
    uint64_t CSR_STLBPS;
    uint64_t CSR_RVACFG;
    uint64_t CSR_CPUID;
    uint64_t CSR_PRCFG1;
    uint64_t CSR_PRCFG2;
    uint64_t CSR_PRCFG3;
    uint64_t CSR_SAVE0;
    uint64_t CSR_SAVE1;
    uint64_t CSR_SAVE2;
    uint64_t CSR_SAVE3;
    uint64_t CSR_SAVE4;
    uint64_t CSR_SAVE5;
    uint64_t CSR_SAVE6;
    uint64_t CSR_SAVE7;
    uint64_t CSR_TMID;
    uint64_t CSR_TCFG;
    uint64_t CSR_TVAL;
    uint64_t CSR_CNTC;
    uint64_t CSR_TINTCLR;
    uint64_t CSR_LLBCTL;
    uint64_t CSR_IMPCTL1;
    uint64_t CSR_IMPCTL2;
    uint64_t CSR_TLBRENTRY;
    uint64_t CSR_TLBRBADV;
    uint64_t CSR_TLBRERA;
    uint64_t CSR_TLBRSAVE;
    uint64_t CSR_TLBRELO0;
    uint64_t CSR_TLBRELO1;
    uint64_t CSR_TLBREHI;
    uint64_t CSR_TLBRPRMD;
    uint64_t CSR_MERRCTL;
    uint64_t CSR_MERRINFO;
    uint64_t CSR_MERRINFO1;
    uint64_t CSR_MERRENT;
    uint64_t CSR_MERRERA;
    uint64_t CSR_MERRSAVE;
    uint64_t CSR_CTAG;
    uint64_t CSR_DMWIN0;
    uint64_t CSR_DMWIN1;
    uint64_t CSR_DMWIN2;
    uint64_t CSR_DMWIN3;
    uint64_t CSR_PERFCTRL0;
    uint64_t CSR_PERFCNTR0;
    uint64_t CSR_PERFCTRL1;
    uint64_t CSR_PERFCNTR1;
    uint64_t CSR_PERFCTRL2;
    uint64_t CSR_PERFCNTR2;
    uint64_t CSR_PERFCTRL3;
    uint64_t CSR_PERFCNTR3;
    uint64_t CSR_MWPC;
    uint64_t CSR_MWPS;
    uint64_t CSR_DB0ADDR;
    uint64_t CSR_DB0MASK;
    uint64_t CSR_DB0CTL;
    uint64_t CSR_DB0ASID;
    uint64_t CSR_DB1ADDR;
    uint64_t CSR_DB1MASK;
    uint64_t CSR_DB1CTL;
    uint64_t CSR_DB1ASID;
    uint64_t CSR_DB2ADDR;
    uint64_t CSR_DB2MASK;
    uint64_t CSR_DB2CTL;
    uint64_t CSR_DB2ASID;
    uint64_t CSR_DB3ADDR;
    uint64_t CSR_DB3MASK;
    uint64_t CSR_DB3CTL;
    uint64_t CSR_DB3ASID;
    uint64_t CSR_FWPC;
    uint64_t CSR_FWPS;
    uint64_t CSR_IB0ADDR;
    uint64_t CSR_IB0MASK;
    uint64_t CSR_IB0CTL;
    uint64_t CSR_IB0ASID;
    uint64_t CSR_IB1ADDR;
    uint64_t CSR_IB1MASK;
    uint64_t CSR_IB1CTL;
    uint64_t CSR_IB1ASID;
    uint64_t CSR_IB2ADDR;
    uint64_t CSR_IB2MASK;
    uint64_t CSR_IB2CTL;
    uint64_t CSR_IB2ASID;
    uint64_t CSR_IB3ADDR;
    uint64_t CSR_IB3MASK;
    uint64_t CSR_IB3CTL;
    uint64_t CSR_IB3ASID;
    uint64_t CSR_IB4ADDR;
    uint64_t CSR_IB4MASK;
    uint64_t CSR_IB4CTL;
    uint64_t CSR_IB4ASID;
    uint64_t CSR_IB5ADDR;
    uint64_t CSR_IB5MASK;
    uint64_t CSR_IB5CTL;
    uint64_t CSR_IB5ASID;
    uint64_t CSR_IB6ADDR;
    uint64_t CSR_IB6MASK;
    uint64_t CSR_IB6CTL;
    uint64_t CSR_IB6ASID;
    uint64_t CSR_IB7ADDR;
    uint64_t CSR_IB7MASK;
    uint64_t CSR_IB7CTL;
    uint64_t CSR_IB7ASID;
    uint64_t CSR_DBG;
    uint64_t CSR_DERA;
    uint64_t CSR_DESAVE;

#ifndef CONFIG_USER_ONLY
    uint64_t      stlb_mask;
    uint32_t      stlb_size; /* at most : 8 * 256 = 2048 */
    uint32_t      mtlb_size; /* at most : 64 */
    loongarch_tlb tlb[LOONGARCH_TLB_MAX];
    int           tlbfill;
#endif
    void *irq[N_IRQS];
    QEMUTimer *timer; /* Internal timer */
};

/**
 * LoongArchCPU:
 * @env: #CPULoongArchState
 *
 * A LoongArch CPU.
 */
struct LoongArchCPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUNegativeOffsetState neg;
    CPULoongArchState env;
    uint32_t id;
    int32_t core_id;
    int32_t node_id; /* NUMA node this CPU belongs to */
};

#define TYPE_LOONGARCH_CPU "loongarch64-cpu"

OBJECT_DECLARE_TYPE(LoongArchCPU, LoongArchCPUClass,
                    LOONGARCH_CPU)

/**
 * LoongArchCPUClass:
 * @parent_realize: The parent class' realize handler.
 * @parent_reset: The parent class' reset handler.
 *
 * A LoongArch CPU model.
 */
struct LoongArchCPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    DeviceRealize parent_realize;
    DeviceReset parent_reset;
};

static inline void cpu_get_tb_cpu_state(CPULoongArchState *env,
                                        target_ulong *pc,
                                        target_ulong *cs_base,
                                        uint32_t *flags)
{
    *pc = env->pc;
    *cs_base = 0;
    *flags = 0;
}

void loongarch_cpu_list(void);

#define cpu_list loongarch_cpu_list

#define MMU_USER_IDX 3

static inline int cpu_mmu_index(CPULoongArchState *env, bool ifetch)
{
#ifdef CONFIG_USER_ONLY
    return MMU_USER_IDX;
#else
    return FIELD_EX64(env->CSR_CRMD, CSR_CRMD, PLV);
#endif
}

typedef CPULoongArchState CPUArchState;
typedef LoongArchCPU ArchCPU;

#include "exec/cpu-all.h"

/* Exceptions */
enum {
    EXCP_NONE          = -1,
    EXCP_ADE           = 0,
    EXCP_SYSCALL,
    EXCP_BREAK,
    EXCP_INE,
    EXCP_FPE,
    EXCP_TLBL,
    EXCP_TLBS,
    EXCP_INST_NOTAVAIL,
    EXCP_TLBM,
    EXCP_TLBPE,
    EXCP_TLBNX,
    EXCP_TLBNR,
    EXCP_EXT_INTERRUPT,
    EXCP_DBP,
    EXCP_IBE,
    EXCP_DBE,

    EXCP_LAST = EXCP_DBE,
};

#define CPU_INTERRUPT_WAKE CPU_INTERRUPT_TGT_INT_0

#define LOONGARCH_CPU_TYPE_SUFFIX "-" TYPE_LOONGARCH_CPU
#define LOONGARCH_CPU_TYPE_NAME(model) model LOONGARCH_CPU_TYPE_SUFFIX
#define CPU_RESOLVING_TYPE TYPE_LOONGARCH_CPU

void cpu_loongarch_clock_init(LoongArchCPU *cpu);
uint64_t cpu_loongarch_get_stable_counter(CPULoongArchState *env);
uint64_t cpu_loongarch_get_stable_timer_ticks(CPULoongArchState *env);
void cpu_loongarch_store_stable_timer_config(CPULoongArchState *env,
                                             uint64_t value);
#endif /* LOONGARCH_CPU_H */
