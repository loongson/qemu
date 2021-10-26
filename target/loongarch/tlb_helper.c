/*
 * QEMU LoongArch TLB helpers for qemu
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"

#include "cpu.h"
#include "internals.h"
#include "exec/helper-proto.h"
#include "exec/exec-all.h"
#include "exec/cpu_ldst.h"
#include "exec/log.h"
#include "cpu-csr.h"

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

/* TLB address map */
static int loongarch_map_address_tlb_entry(
    CPULoongArchState    *env,
    hwaddr          *physical,
    int             *prot,
    target_ulong    address,
    int             access_type,
    loongarch_tlb   *tlb)
{
    uint64_t mask = tlb->PageMask;
    int n   = !!(address & mask & ~(mask >> 1));
    uint64_t plv = FIELD_EX64(env->CSR_CRMD, CSR_CRMD, PLV);

    /* Check access rights */
    if (!(n ? tlb->V1 : tlb->V0)) {
        return TLBRET_INVALID;
    }

    if (access_type == MMU_INST_FETCH && (n ? tlb->NX1 : tlb->NX0)) {
        return TLBRET_XI;
    }

    if (access_type == MMU_DATA_LOAD  && (n ? tlb->NR1 : tlb->NR0)) {
        return TLBRET_RI;
    }

    if (n) {
        if (((tlb->RPLV1 == 0) && (plv > tlb->PLV1)) ||
            ((tlb->RPLV1 == 1) && (plv != tlb->PLV1))) {
            return TLBRET_PE;
        }
    } else {
        if (((tlb->RPLV0 == 0) && (plv > tlb->PLV0)) ||
            ((tlb->RPLV0 == 1) && (plv != tlb->PLV0))) {
            return TLBRET_PE;
        }
    }

    if ((access_type == MMU_DATA_STORE) && !(n ? tlb->D1 : tlb->D0)) {
        return TLBRET_DIRTY;
    }

    /*
     *         PPN     address
     *  4 KB: [47:13]   [12;0]
     * 16 KB: [47:15]   [14:0]
     */
    if (n) {
        *physical = tlb->PPN1 | (address & (mask >> 1));
    } else {
        *physical = tlb->PPN0 | (address & (mask >> 1));
    }
    *prot = PAGE_READ;
    if (n ? tlb->D1 : tlb->D0) {
        *prot |= PAGE_WRITE;
    }
    if (!(n ? tlb->NX1 : tlb->NX0)) {
        *prot |= PAGE_EXEC;
    }
    return TLBRET_MATCH;
}

/* Loongarch 3A5K -style MMU emulation */
int loongarch_map_address(CPULoongArchState *env, hwaddr *physical, int *prot,
                       target_ulong address, MMUAccessType access_type)
{
    uint16_t        asid = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);
    int             i;
    loongarch_tlb   *tlb;

    int stlb_size = env->stlb_size;
    int mtlb_size = env->mtlb_size;

    int stlb_idx;

    uint64_t mask;
    uint64_t vpn;   /* address to map */
    uint64_t tag;   /* address in TLB entry */

    /* search MTLB */
    for (i = stlb_size; i < stlb_size + mtlb_size; ++i) {
        tlb = &env->tlb[i];
        mask = tlb->PageMask;

        vpn = address & 0xffffffffe000 & ~mask;
        tag = tlb->VPN & ~mask;

        if ((tlb->G == 1 || tlb->ASID == asid) && vpn == tag && tlb->NE != 1) {
            return loongarch_map_address_tlb_entry(env, physical, prot,
                                                address, access_type, tlb);
        }
    }

    if (stlb_size == 0) {
        return TLBRET_NOMATCH;
    }

    /* search STLB */
    mask = env->stlb_mask;
    vpn = address & 0xffffffffe000 & ~mask;

    stlb_idx = (address & 0xffffffffc000) >> 15; /* 16 KB */
    stlb_idx = stlb_idx & 0xff; /* [0,255] */

    for (i = 0; i < 8; ++i) {
        tlb = &env->tlb[i * 256 + stlb_idx];
        tag = tlb->VPN & ~mask;

        if ((tlb->G == 1 || tlb->ASID == asid) && vpn == tag && tlb->NE != 1) {
            return loongarch_map_address_tlb_entry(env, physical, prot,
                                                address, access_type, tlb);
        }
    }

    return TLBRET_NOMATCH;
}

static int get_physical_address(CPULoongArchState *env, hwaddr *physical,
                                int *prot, target_ulong real_address,
                                MMUAccessType access_type, int mmu_idx)
{
    int user_mode = mmu_idx == LOONGARCH_HFLAG_UM;
    int kernel_mode = !user_mode;
    unsigned plv, base_c, base_v, tmp;
    uint64_t pg = FIELD_EX64(env->CSR_CRMD, CSR_CRMD, PG);

    /* effective address (modified for KVM T&E kernel segments) */
    target_ulong address = real_address;

    /* Check PG */
    if (!pg) {
        /* DA mode */
        *physical = address & 0xffffffffffffUL;
        *prot = PAGE_READ | PAGE_WRITE | PAGE_EXEC;
        return TLBRET_MATCH;
    }

    plv = kernel_mode | (user_mode << 3);
    base_v = address >> CSR_DMW_BASE_SH;
    /* Check direct map window 0 */
    base_c = env->CSR_DMWIN0 >> CSR_DMW_BASE_SH;
    if ((plv & env->CSR_DMWIN0) && (base_c == base_v)) {
        *physical = dmwin_va2pa(address);
        *prot = PAGE_READ | PAGE_WRITE | PAGE_EXEC;
        return TLBRET_MATCH;
    }
    /* Check direct map window 1 */
    base_c = env->CSR_DMWIN1 >> CSR_DMW_BASE_SH;
    if ((plv & env->CSR_DMWIN1) && (base_c == base_v)) {
        *physical = dmwin_va2pa(address);
        *prot = PAGE_READ | PAGE_WRITE | PAGE_EXEC;
        return TLBRET_MATCH;
    }
    /* Check valid extension */
    tmp = address >> 47;
    if (!(tmp == 0 || tmp == 0x1ffff)) {
        return TLBRET_BADADDR;
    }
    /* mapped address */
    return loongarch_map_address(env, physical, prot, real_address, access_type);
}

hwaddr loongarch_cpu_get_phys_page_debug(CPUState *cs, vaddr addr)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;
    hwaddr phys_addr;
    int prot;

    if (get_physical_address(env, &phys_addr, &prot, addr, MMU_DATA_LOAD,
                             cpu_mmu_index(env, false)) != 0) {
        return -1;
    }
    return phys_addr;
}

static void raise_mmu_exception(CPULoongArchState *env, target_ulong address,
                                MMUAccessType access_type, int tlb_error)
{
    CPUState *cs = env_cpu(env);
    int exception = 0;

    switch (tlb_error) {
    default:
    case TLBRET_BADADDR:
        exception = EXCP_ADE;
        break;
    case TLBRET_NOMATCH:
        /* No TLB match for a mapped address */
        if (access_type == MMU_DATA_LOAD) {
            exception = EXCP_TLBL;
        } else if (access_type == MMU_DATA_STORE) {
            exception = EXCP_TLBS;
        } else if (access_type == MMU_INST_FETCH) {
            exception = EXCP_INST_NOTAVAIL;
        }
        env->tlbfill = 1;
        break;
    case TLBRET_INVALID:
        /* TLB match with no valid bit */
        if (access_type == MMU_DATA_LOAD) {
            exception = EXCP_TLBL;
        } else if (access_type == MMU_DATA_STORE) {
            exception = EXCP_TLBS;
        } else if (access_type == MMU_INST_FETCH) {
            exception = EXCP_INST_NOTAVAIL;
        }
        break;
    case TLBRET_DIRTY:
        /* TLB match but 'D' bit is cleared */
        exception = EXCP_TLBM;
        break;
    case TLBRET_XI:
        /* Execute-Inhibit Exception */
        exception = EXCP_TLBNX;
        break;
    case TLBRET_RI:
        /* Read-Inhibit Exception */
        exception = EXCP_TLBNR;
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
        return;
    }

    /* Raise exception */
    if (!FIELD_EX64(env->CSR_DBG, CSR_DBG, DST)) {
        env->CSR_BADV = address;
    }
    cs->exception_index = exception;

    env->CSR_TLBEHI = address & (TARGET_PAGE_MASK << 1);
}

void cpu_loongarch_tlb_flush(CPULoongArchState *env)
{
    /* Flush qemu's TLB and discard all shadowed entries. */
    tlb_flush(env_cpu(env));
}

static void loongarch_invalidate_tlb_entry(CPULoongArchState *env,
                                           loongarch_tlb *tlb)
{
    CPUState *cs = env_cpu(env);
    target_ulong    addr;
    target_ulong    end;
    target_ulong    mask;

    mask = tlb->PageMask; /* 000...000111...111 */

    if (tlb->V0) {
        addr = tlb->VPN & ~mask;    /* xxx...xxx[0]000..0000 */
        end = addr | (mask >> 1);   /* xxx...xxx[0]111..1111 */
        while (addr < end) {
            tlb_flush_page(cs, addr);
            addr += TARGET_PAGE_SIZE;
        }
    }

    if (tlb->V1) {
        /* xxx...xxx[1]000..0000 */
        addr = (tlb->VPN & ~mask) | ((mask >> 1) + 1);
        end = addr | mask;              /* xxx...xxx[1]111..1111 */
        while (addr - 1 < end) {
            tlb_flush_page(cs, addr);
            addr += TARGET_PAGE_SIZE;
        }
    }
}

void loongarch_invalidate_tlb(CPULoongArchState *env, int idx)
{
    loongarch_tlb *tlb;
    uint16_t asid = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);

    tlb = &env->tlb[idx];
    if (tlb->G == 0 && tlb->ASID != asid) {
        return;
    }
    loongarch_invalidate_tlb_entry(env, tlb);
}

static uint64_t loongarch_pagesize_to_mask(int pagesize)
{
    /* 4KB - 1GB */
    if (pagesize < 12 && pagesize > 30) {
        qemu_log_mask(CPU_LOG_MMU, "unsupported page size %d\n", pagesize);
    }

    return (1 << (pagesize + 1)) - 1;
}

static void loongarch_fill_tlb_entry(CPULoongArchState *env,
                                     loongarch_tlb *tlb, int is_stlb)
{
    uint64_t page_mask;  /* 0000...00001111...1111 */
    uint32_t page_size;
    uint64_t entryhi;
    uint64_t lo0, lo1;

    if (FIELD_EX64(env->CSR_TLBRERA, CSR_TLBRERA, ISTLBR)) {
	page_size = FIELD_EX64(env->CSR_TLBREHI, CSR_TLBREHI, PS);
        entryhi = env->CSR_TLBREHI;
        lo0 = env->CSR_TLBRELO0;
        lo1 = env->CSR_TLBRELO1;
    } else {
        page_size = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, PS);
        entryhi = env->CSR_TLBEHI;
        lo0 = env->CSR_TLBELO0;
        lo1 = env->CSR_TLBELO1;
    }

    if (page_size == 0) {
        qemu_log_mask(CPU_LOG_MMU, "page size is 0\n");
    }

    /*
     *       15-12  11-8   7-4    3-0
     *  4KB: 0001   1111   1111   1111 // double 4KB  mask [12:0]
     * 16KB: 0111   1111   1111   1111 // double 16KB mask [14:0]
     */
    if (is_stlb) {
        page_mask = env->stlb_mask;
    } else {
        page_mask = loongarch_pagesize_to_mask(page_size);
    }

    tlb->VPN = entryhi & 0xffffffffe000 & ~page_mask;

    tlb->ASID = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);
    tlb->NE = 0;
    tlb->G = FIELD_EX64(env->CSR_TLBELO0, CSR_TLBELO0, G) &
             FIELD_EX64(env->CSR_TLBELO1, CSR_TLBELO1, G);

    tlb->PageMask = page_mask;
    tlb->PageSize = page_size;

    tlb->V0 = FIELD_EX64(lo0, CSR_TLBELO0, V);      /* [0] */
    tlb->D0 = FIELD_EX64(lo0, CSR_TLBELO0, D);      /* [1] */
    tlb->PLV0 = FIELD_EX64(lo0, CSR_TLBELO0, PLV);  /* [3:2] */
    tlb->MAT0 = FIELD_EX64(lo0, CSR_TLBELO0, MAT);  /* [5:4] */
    tlb->PPN0 = (lo0 & 0xfffffffff000 & ~(page_mask >> 1));
    tlb->NR0 = FIELD_EX64(lo0, CSR_TLBELO0, NR);    /* [61] */
    tlb->NX0 = FIELD_EX64(lo0, CSR_TLBELO0, NX);    /* [62] */
    tlb->RPLV0 = FIELD_EX64(lo0, CSR_TLBELO0, RPLV);/* [63] */

    tlb->V1 = FIELD_EX64(lo1, CSR_TLBELO1, V);      /* [0] */
    tlb->D1 = FIELD_EX64(lo1, CSR_TLBELO1, D);      /* [1] */
    tlb->PLV1 = FIELD_EX64(lo1, CSR_TLBELO1, PLV);  /* [3:2] */
    tlb->MAT1 = FIELD_EX64(lo1, CSR_TLBELO1, MAT);  /* [5:4] */
    tlb->PPN1 = (lo1 & 0xfffffffff000 & ~(page_mask >> 1));
    tlb->NR1 = FIELD_EX64(lo1, CSR_TLBELO1, NR);    /* [61] */
    tlb->NX1 = FIELD_EX64(lo1, CSR_TLBELO1, NX);    /* [62] */
    tlb->RPLV1 = FIELD_EX64(lo1, CSR_TLBELO1, RPLV);/* [63] */
}

static void loongarch_fill_tlb(CPULoongArchState *env, int idx, bool tlbwr)
{
    loongarch_tlb *tlb;

    tlb = &env->tlb[idx];
    if (tlbwr) {
        if (FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, NE)) {
            tlb->NE = 1;
            return;
        }
    }

    if (idx < 2048) {
        loongarch_fill_tlb_entry(env, tlb, 1);
    } else {
        loongarch_fill_tlb_entry(env, tlb, 0);
    }
}

/* return random value in [low, high] */
static uint32_t cpu_loongarch_get_random_loongarch_tlb(uint32_t low, uint32_t high)
{
    static uint32_t seed = 5;
    static uint32_t prev_idx;
    uint32_t idx;
    uint32_t nb_rand_tlb = high - low + 1;

    do {
        seed = 1103515245 * seed + 12345;
        idx = (seed >> 16) % nb_rand_tlb + low;
    } while (idx == prev_idx);
    prev_idx = idx;

    return idx;
}

void helper_tlbsrch(CPULoongArchState *env)
{
    uint64_t mask;
    uint64_t vpn;
    uint64_t tag;
    uint16_t asid;

    int stlb_size = env->stlb_size;
    int mtlb_size = env->mtlb_size;
    int i;
    loongarch_tlb *tlb;
    asid = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);

    /* search MTLB + STLB */
    for (i = 0; i < stlb_size + mtlb_size; ++i) {
        tlb = &env->tlb[i];
        mask = tlb->PageMask;
        vpn = env->CSR_TLBEHI & 0xffffffffe000 & ~mask;
        tag = tlb->VPN & ~mask;

        if ((tlb->G == 1 || tlb->ASID == asid) && vpn == tag && tlb->NE != 1) {
            env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX,
                                         INDEX, (i & 0xfff));
            env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX,
                                         PS, (tlb->PageSize & 0x3f));
            return;
        }
    }

    env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX, NE, 1);
}

void helper_tlbrd(CPULoongArchState *env)
{
    loongarch_tlb    *tlb;
    int             idx;
    uint16_t        asid;

    idx = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, INDEX);
    tlb = &env->tlb[idx];

    asid = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);

    if (asid != tlb->ASID) {
        cpu_loongarch_tlb_flush(env);
    }

    if (tlb->NE) {
        /* invalid TLB entry */
        env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX, NE, 1);
        env->CSR_TLBEHI = 0;
        env->CSR_TLBELO0 = 0;
        env->CSR_TLBELO1 = 0;
    } else {
        /* valid TLB entry */
        env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX,
                                     INDEX, (idx & 0xfff));
        env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX,
                                     PS, (tlb->PageSize & 0x3f));
        env->CSR_TLBEHI = tlb->VPN;

        env->CSR_TLBELO0 = FIELD_DP64(0, CSR_TLBELO0, V, tlb->V0);
        env->CSR_TLBELO0 = FIELD_DP64(env->CSR_TLBELO0, CSR_TLBELO0, D, tlb->D0);
        env->CSR_TLBELO0 = FIELD_DP64(env->CSR_TLBELO0, CSR_TLBELO0, PLV, tlb->PLV0);
        env->CSR_TLBELO0 = FIELD_DP64(env->CSR_TLBELO0, CSR_TLBELO0, MAT, tlb->MAT0);
        env->CSR_TLBELO0 = FIELD_DP64(env->CSR_TLBELO0, CSR_TLBELO0, G, tlb->G);
        env->CSR_TLBELO0 = FIELD_DP64(env->CSR_TLBELO0, CSR_TLBELO0, PPN, tlb->PPN0);
        env->CSR_TLBELO0 = FIELD_DP64(env->CSR_TLBELO0, CSR_TLBELO0, NR, tlb->NR0);
        env->CSR_TLBELO0 = FIELD_DP64(env->CSR_TLBELO0, CSR_TLBELO0, NX, tlb->NX0);
        env->CSR_TLBELO0 = FIELD_DP64(env->CSR_TLBELO0, CSR_TLBELO0, RPLV, tlb->RPLV0);

        env->CSR_TLBELO1 = FIELD_DP64(0, CSR_TLBELO1, V, tlb->V1);
        env->CSR_TLBELO1 = FIELD_DP64(env->CSR_TLBELO1, CSR_TLBELO1, D, tlb->D1);
        env->CSR_TLBELO1 = FIELD_DP64(env->CSR_TLBELO1, CSR_TLBELO1, PLV, tlb->PLV1);
        env->CSR_TLBELO1 = FIELD_DP64(env->CSR_TLBELO1, CSR_TLBELO1, MAT, tlb->MAT1);
        env->CSR_TLBELO1 = FIELD_DP64(env->CSR_TLBELO1, CSR_TLBELO1, G, tlb->G);
        env->CSR_TLBELO1 = FIELD_DP64(env->CSR_TLBELO1, CSR_TLBELO1, PPN, tlb->PPN1);
        env->CSR_TLBELO1 = FIELD_DP64(env->CSR_TLBELO1, CSR_TLBELO1, NR, tlb->NR1);
        env->CSR_TLBELO1 = FIELD_DP64(env->CSR_TLBELO1, CSR_TLBELO1, NX, tlb->NX1);
        env->CSR_TLBELO1 = FIELD_DP64(env->CSR_TLBELO1, CSR_TLBELO1, RPLV, tlb->RPLV1);

        env->CSR_ASID  = FIELD_DP64(env->CSR_ASID, CSR_ASID, ASID, tlb->ASID);
    }
}

void helper_tlbwr(CPULoongArchState *env)
{
    int idx = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, INDEX); /* 0-11 */

    loongarch_invalidate_tlb(env, idx);
    loongarch_fill_tlb(env, idx, true);
}

void helper_tlbfill(CPULoongArchState *env)
{
    uint64_t mask;
    uint64_t address;
    int idx;
    int set, stlb_idx;

    uint64_t entryhi;
    uint32_t pagesize;

    if (FIELD_EX64(env->CSR_TLBRERA, CSR_TLBRERA, ISTLBR)) {
        entryhi = env->CSR_TLBREHI;
        pagesize = FIELD_EX64(env->CSR_TLBREHI, CSR_TLBREHI, PS);
    } else {
        entryhi = env->CSR_TLBEHI;
        pagesize = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, PS);
    }

    uint32_t stlb_size = env->stlb_size;
    uint32_t mtlb_size = env->mtlb_size;

    mask = loongarch_pagesize_to_mask(pagesize);

    if (mask == env->stlb_mask && env->stlb_size > 0) {
        /* only write into STLB */
        address = entryhi & 0xffffffffe000; /* [47:13] */

        /* choose one set ramdomly */
        set = cpu_loongarch_get_random_loongarch_tlb(0, 7);

        /* index in one set */
        stlb_idx = (address >> 15) & 0xff; /* [0,255] */

        idx = set * 256 + stlb_idx;
    } else {
        /* only write into MTLB */
        idx = cpu_loongarch_get_random_loongarch_tlb(
                stlb_size, stlb_size + mtlb_size - 1);
    }

    loongarch_invalidate_tlb(env, idx);
    loongarch_fill_tlb(env, idx, false);
}

void helper_tlbclr(CPULoongArchState *env)
{
    int i;
    uint16_t asid;
    int msize, ssize, index;
    loongarch_tlb *tlb;

    asid = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);
    msize = env->mtlb_size;
    ssize = env->stlb_size;
    index = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, INDEX);

    if (index < ssize) {
        /* STLB. One line per operation */
        for (i = 0; i < 8; i++) {
            tlb = &env->tlb[i * 256 + (index % 256)];
            if (!tlb->G && tlb->ASID == asid) {
                tlb->NE = 1;
            }
        }
    } else if (index < (ssize + msize)) {
        /* MTLB. All entries */
        for (i = ssize; i < ssize + msize; i++) {
            tlb = &env->tlb[i];
            if (!tlb->G && tlb->ASID == asid) {
                tlb->NE = 1;
            }
        }
    }

    cpu_loongarch_tlb_flush(env);
}

void helper_tlbflush(CPULoongArchState *env)
{
    int i;
    int msize, ssize, index;

    msize = env->mtlb_size;
    ssize = env->stlb_size;
    index = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, INDEX);

    if (index < ssize) {
        /* STLB. One line per operation */
        for (i = 0; i < 8; i++) {
            int idx = i * 256 + (index % 256);
            env->tlb[idx].NE = 1;
        }
    } else if (index < (ssize + msize)) {
        /* MTLB. All entries */
        for (i = ssize; i < ssize + msize; i++) {
            env->tlb[i].NE = 1;
        }
    }

    cpu_loongarch_tlb_flush(env);
}

void helper_invtlb(CPULoongArchState *env, target_ulong addr,
                   target_ulong info, target_ulong op)
{
    uint32_t asid = info & 0x3ff;
    int i;

    switch (op) {
    case 0:
    case 1:
        for (i = 0; i < LOONGARCH_TLB_MAX; i++) {
            env->tlb[i].NE = 1;
        }
        break;
    case 2:
        for (i = 0; i < LOONGARCH_TLB_MAX; i++) {
            loongarch_tlb *tlb = &env->tlb[i];

            if (tlb->G) {
                tlb->NE = 1;
            }
        }
        break;
    case 3:
        for (i = 0; i < LOONGARCH_TLB_MAX; i++) {
            loongarch_tlb *tlb = &env->tlb[i];

            if (!tlb->G) {
                tlb->NE = 1;
            }
        }
        break;
    case 4:
        for (i = 0; i < LOONGARCH_TLB_MAX; i++) {
            loongarch_tlb *tlb = &env->tlb[i];

            if (!tlb->G && tlb->ASID == asid) {
                tlb->NE = 1;
            }
        }
        break;
    case 5:
        for (i = 0; i < LOONGARCH_TLB_MAX; i++) {
            loongarch_tlb *tlb = &env->tlb[i];
            uint64_t vpn = addr & 0xffffffffe000 & ~tlb->PageMask;

            if (!tlb->G && tlb->ASID == asid && vpn == tlb->VPN) {
                tlb->NE = 1;
            }
        }
        break;
    case 6:
        for (i = 0; i < LOONGARCH_TLB_MAX; i++) {
            loongarch_tlb *tlb = &env->tlb[i];
            uint64_t vpn = addr & 0xffffffffe000 & ~tlb->PageMask;

            if ((tlb->G || tlb->ASID == asid) && vpn == tlb->VPN) {
                tlb->NE = 1;
            }
        }
        break;
    default:
        do_raise_exception(env, EXCP_INE, GETPC());
    }

    cpu_loongarch_tlb_flush(env);
}

void loongarch_mmu_init(CPULoongArchState *env)
{
    /* number of MTLB */
    env->mtlb_size = 64;

    /* number of STLB */
    env->stlb_size = 2048;
    env->stlb_mask = (1 << 15) - 1; /* 16 KB */
    /*
     * page_size    |        stlb_mask         | party field
     * ----------------------------------------------------------------
     *   4 KB = 12  | ( 1 << 13 ) - 1 = [12:0] |   [12]
     *  16 KB = 14  | ( 1 << 15 ) - 1 = [14:0] |   [14]
     *  64 KB = 16  | ( 1 << 17 ) - 1 = [16:0] |   [16]
     * 256 KB = 18  | ( 1 << 19 ) - 1 = [18:0] |   [18]
     *   1 MB = 20  | ( 1 << 21 ) - 1 = [20:0] |   [20]
     *   4 MB = 22  | ( 1 << 23 ) - 1 = [22:0] |   [22]
     *  16 MB = 24  | ( 1 << 25 ) - 1 = [24:0] |   [24]
     *  64 MB = 26  | ( 1 << 27 ) - 1 = [26:0] |   [26]
     * 256 MB = 28  | ( 1 << 29 ) - 1 = [28:0] |   [28]
     *   1 GB = 30  | ( 1 << 31 ) - 1 = [30:0] |   [30]
     * ----------------------------------------------------------------
     *  take party field index as @n. eg. For 16 KB, n = 14
     * ----------------------------------------------------------------
     *  tlb->VPN = TLBEHI & 0xffffffffe000[47:13] & ~mask = [47:n+1]
     *  tlb->PPN = TLBLO0 & 0xffffffffe000[47:13] & ~mask = [47:n+1]
     *  tlb->PPN = TLBLO1 & 0xffffffffe000[47:13] & ~mask = [47:n+1]
     * ----------------------------------------------------------------
     *  On mapping :
     *  >   vpn = address  & 0xffffffffe000[47:13] & ~mask = [47:n+1]
     *  >   tag = tlb->VPN & ~mask                         = [47:n+1]
     * ----------------------------------------------------------------
     * physical address = [47:n+1]  |  [n:0]
     * physical address = tlb->PPN0 | (address & mask)
     * physical address = tlb->PPN1 | (address & mask)
     */

    int i;
    for (i = 0; i < LOONGARCH_TLB_MAX; i++) {
        env->tlb[i].NE = 1;
    }
}

bool loongarch_cpu_tlb_fill(CPUState *cs, vaddr address, int size,
                            MMUAccessType access_type, int mmu_idx,
                            bool probe, uintptr_t retaddr)
{
    LoongArchCPU *cpu = LOONGARCH_CPU(cs);
    CPULoongArchState *env = &cpu->env;
    hwaddr physical;
    int prot;
    int ret = TLBRET_BADADDR;

    /* data access */
    /* XXX: put correct access by using cpu_restore_state() correctly */
    ret = get_physical_address(env, &physical, &prot, address,
                               access_type, mmu_idx);
    switch (ret) {
    case TLBRET_MATCH:
        qemu_log_mask(CPU_LOG_MMU,
                      "%s address=%" VADDR_PRIx " physical " TARGET_FMT_plx
                      " prot %d\n", __func__, address, physical, prot);
        break;
    default:
        qemu_log_mask(CPU_LOG_MMU,
                      "%s address=%" VADDR_PRIx " ret %d\n", __func__, address,
                      ret);
        break;
    }
    if (ret == TLBRET_MATCH) {
        tlb_set_page(cs, address & TARGET_PAGE_MASK,
                     physical & TARGET_PAGE_MASK, prot,
                     mmu_idx, TARGET_PAGE_SIZE);
        return true;
    }
    if (probe) {
        return false;
    } else {
        raise_mmu_exception(env, address, access_type, ret);
        do_raise_exception(env, cs->exception_index, retaddr);
    }
}
