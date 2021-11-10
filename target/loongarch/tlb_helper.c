/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * QEMU LoongArch TLB helpers for qemu
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 */

#include "qemu/osdep.h"
#include "qemu/guest-random.h"

#include "cpu.h"
#include "internals.h"
#include "exec/helper-proto.h"
#include "exec/exec-all.h"
#include "exec/cpu_ldst.h"
#include "exec/log.h"
#include "cpu-csr.h"

enum {
    TLBRET_MATCH = 0,
    TLBRET_BADADDR = 1,
    TLBRET_NOMATCH = 2,
    TLBRET_INVALID = 3,
    TLBRET_DIRTY = 4,
    TLBRET_RI = 5,
    TLBRET_XI = 6,
    TLBRET_PE = 7,
};

/* TLB address map */
static int loongarch_map_tlb_entry(CPULoongArchState *env, hwaddr *physical,
                                   int *prot, target_ulong address,
                                   int access_type, int index, int mmu_idx)
{
    loongarch_tlb *tlb = &env->tlb[index];
    uint64_t plv = mmu_idx;
    uint64_t tlb_entry, tlb_ppn;
    uint8_t tlb_ps, n, tlb_v, tlb_d, tlb_plv, tlb_nx, tlb_nr, tlb_rplv;

    if (index > env->stlb_size) {
        tlb_ps = FIELD_EX64(tlb->tlb_misc, TLB_MISC, PS);
    } else {
        tlb_ps = FIELD_EX64(env->CSR_STLBPS, CSR_STLBPS, PS);
    }
    n = (address >> tlb_ps) & 0x1;/* Odd or even */

    tlb_entry = n ? tlb->tlb_entry1 : tlb->tlb_entry0;
    tlb_v = FIELD_EX64(tlb_entry, TLBENTRY, V);
    tlb_d = FIELD_EX64(tlb_entry, TLBENTRY, D);
    tlb_plv = FIELD_EX64(tlb_entry, TLBENTRY, PLV);
    tlb_ppn = FIELD_EX64(tlb_entry, TLBENTRY, PPN);
    tlb_nx = FIELD_EX64(tlb_entry, TLBENTRY, NX);
    tlb_nr = FIELD_EX64(tlb_entry, TLBENTRY, NR);
    tlb_rplv = FIELD_EX64(tlb_entry, TLBENTRY, RPLV);

    /* Check access rights */
    if (!tlb_v) {
        return TLBRET_INVALID;
    }

    if (access_type == MMU_INST_FETCH && tlb_nx) {
        return TLBRET_XI;
    }

    if (access_type == MMU_DATA_LOAD && tlb_nr) {
        return TLBRET_RI;
    }

    if (((tlb_rplv == 0) && (plv > tlb_plv)) ||
        ((tlb_rplv == 1) && (plv != tlb_plv))) {
        return TLBRET_PE;
    }

    if ((access_type == MMU_DATA_STORE) && !tlb_d) {
        return TLBRET_DIRTY;
    }

    /*
     * tlb_entry contains ppn[47:12] while 16KB ppn is [47:15]
     * need adjust.
     */
    *physical = (tlb_ppn << R_TLBENTRY_PPN_SHIFT) |
                (address & MAKE_64BIT_MASK(0, tlb_ps));
    *prot = PAGE_READ;
    if (tlb_d) {
        *prot |= PAGE_WRITE;
    }
    if (!tlb_nx) {
        *prot |= PAGE_EXEC;
    }
    return TLBRET_MATCH;
}

/*
 * One tlb entry holds a adjacent odd/even pair, the vpn is the
 * content of the virtual page number divided by 2.So the
 * compare vpn is bit[47:15] for 16KB page. while the vppn
 * field in tlb entry contains bit[47:13], so need adjust.
 * virt_vpn = vaddr[47:13]
 */
static bool loongarch_tlb_search(CPULoongArchState *env, target_ulong vaddr,
                                 int *index)
{
    loongarch_tlb *tlb;
    uint16_t csr_asid, tlb_asid, stlb_idx;
    uint8_t tlb_e, tlb_ps, tlb_g, stlb_ps;
    int i, stlb_size, mtlb_size, compare_shift;
    uint64_t vpn, tlb_vppn;   /* Address to map */

    stlb_size = env->stlb_size;
    mtlb_size = env->mtlb_size;
    csr_asid = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);
    stlb_ps = FIELD_EX64(env->CSR_STLBPS, CSR_STLBPS, PS);
    vpn = (vaddr & TARGET_VIRT_MASK) >> (stlb_ps + 1);
    stlb_idx = vpn & 0xff; /* VA[25:15] <==> TLBIDX.index for 16KB Page */
    compare_shift = stlb_ps + 1 - R_TLB_MISC_VPPN_SHIFT;

    /* Search STLB */
    for (i = 0; i < 8; ++i) {
        tlb = &env->tlb[i * 256 + stlb_idx];
        tlb_e = FIELD_EX64(tlb->tlb_misc, TLB_MISC, E);
        if (tlb_e) {
            tlb_vppn = FIELD_EX64(tlb->tlb_misc, TLB_MISC, VPPN);
            tlb_asid = FIELD_EX64(tlb->tlb_misc, TLB_MISC, ASID);
            tlb_g = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, G);

            if ((tlb_g == 1 || tlb_asid == csr_asid) &&
                (vpn == (tlb_vppn >> compare_shift))) {
                *index = i * 256 + stlb_idx;
                return true;
            }
        }
    }

    /* Search MTLB */
    for (i = stlb_size; i < stlb_size + mtlb_size; ++i) {
        tlb = &env->tlb[i];
        tlb_e = FIELD_EX64(tlb->tlb_misc, TLB_MISC, E);
        if (tlb_e) {
            tlb_vppn = FIELD_EX64(tlb->tlb_misc, TLB_MISC, VPPN);
            tlb_ps = FIELD_EX64(tlb->tlb_misc, TLB_MISC, PS);
            tlb_asid = FIELD_EX64(tlb->tlb_misc, TLB_MISC, ASID);
            tlb_g = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, G);
            compare_shift = tlb_ps + 1 - R_TLB_MISC_VPPN_SHIFT;

            if ((tlb_g == 1 || tlb_asid == csr_asid) &&
                (vpn == (tlb_vppn >> compare_shift))) {
                *index = i;
                return true;
            }
        }
    }
    return false;
}

static int loongarch_map_address(CPULoongArchState *env, hwaddr *physical,
                                 int *prot, target_ulong address,
                                 MMUAccessType access_type, int mmu_idx)
{
    int index, match;

    match = loongarch_tlb_search(env, address, &index);
    if (match) {
        return loongarch_map_tlb_entry(env, physical, prot,
                                       address, access_type, index, mmu_idx);
    }

    return TLBRET_NOMATCH;
}

static int get_physical_address(CPULoongArchState *env, hwaddr *physical,
                                int *prot, target_ulong address,
                                MMUAccessType access_type, int mmu_idx)
{
    int user_mode = mmu_idx == MMU_USER_IDX;
    int kernel_mode = mmu_idx == MMU_KERNEL_IDX;
    uint32_t plv, base_c, base_v;
    int64_t addr_high;
    uint8_t da = FIELD_EX64(env->CSR_CRMD, CSR_CRMD, DA);
    uint8_t pg = FIELD_EX64(env->CSR_CRMD, CSR_CRMD, PG);

    /* Check PG and DA*/
    if (da & !pg) {
        /* DA mode */
        *physical = address & TARGET_PHYS_MASK;
        *prot = PAGE_READ | PAGE_WRITE | PAGE_EXEC;
        return TLBRET_MATCH;
    }

    plv = kernel_mode | (user_mode << R_CSR_DMW_PLV3_SHIFT);
    base_v = address >> TARGET_VIRT_ADDR_SPACE_BITS;
    /* Check direct map window */
    for (int i = 0; i < 4; i++) {
        base_c = env->CSR_DMW[i] >> TARGET_VIRT_ADDR_SPACE_BITS;
        if ((plv & env->CSR_DMW[i]) && (base_c == base_v)) {
            *physical = dmw_va2pa(address);
            *prot = PAGE_READ | PAGE_WRITE | PAGE_EXEC;
            return TLBRET_MATCH;
        }
    }

    /* Check valid extension */
    addr_high = sextract64(address, TARGET_VIRT_ADDR_SPACE_BITS, 16);
    if (!(addr_high == 0 || addr_high == -1)) {
        return TLBRET_BADADDR;
    }
    /* Mapped address */
    return loongarch_map_address(env, physical, prot, address,
                                 access_type, mmu_idx);
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

    switch (tlb_error) {
    default:
    case TLBRET_BADADDR:
        cs->exception_index = EXCCODE_ADEM;
        break;
    case TLBRET_NOMATCH:
        /* No TLB match for a mapped address */
        if (access_type == MMU_DATA_LOAD) {
            cs->exception_index = EXCCODE_PIL;
        } else if (access_type == MMU_DATA_STORE) {
            cs->exception_index = EXCCODE_PIS;
        } else if (access_type == MMU_INST_FETCH) {
            cs->exception_index = EXCCODE_PIF;
        }
        env->CSR_TLBRERA = FIELD_DP64(env->CSR_TLBRERA, CSR_TLBRERA, ISTLBR, 1);
        break;
    case TLBRET_INVALID:
        /* TLB match with no valid bit */
        if (access_type == MMU_DATA_LOAD) {
            cs->exception_index = EXCCODE_PIL;
        } else if (access_type == MMU_DATA_STORE) {
            cs->exception_index = EXCCODE_PIS;
        } else if (access_type == MMU_INST_FETCH) {
            cs->exception_index = EXCCODE_PIF;
        }
        break;
    case TLBRET_DIRTY:
        /* TLB match but 'D' bit is cleared */
        cs->exception_index = EXCCODE_PME;
        break;
    case TLBRET_XI:
        /* Execute-Inhibit Exception */
        cs->exception_index = EXCCODE_PNX;
        break;
    case TLBRET_RI:
        /* Read-Inhibit Exception */
        cs->exception_index = EXCCODE_PNR;
        break;
    case TLBRET_PE:
        /* Privileged Exception */
        cs->exception_index = EXCCODE_PPI;
        break;
    }

    if (tlb_error == TLBRET_NOMATCH) {
        env->CSR_TLBRBADV = address;
        env->CSR_TLBREHI = address & (TARGET_PAGE_MASK << 1);
    } else {
        if (!FIELD_EX64(env->CSR_DBG, CSR_DBG, DST)) {
            env->CSR_BADV = address;
        }
        env->CSR_TLBEHI = address & (TARGET_PAGE_MASK << 1);
   }

}

static void cpu_loongarch_tlb_flush(CPULoongArchState *env)
{
    /* Flush qemu's TLB and discard all shadowed entries. */
    tlb_flush(env_cpu(env));
}

static void loongarch_invalidate_tlb_entry(CPULoongArchState *env,
                                           int index)
{
    target_ulong addr, mask, pagesize;
    uint8_t tlb_ps;
    loongarch_tlb *tlb = &env->tlb[index];

    int mmu_idx = cpu_mmu_index(env, false);
    uint8_t tlb_v0 = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, V);
    uint8_t tlb_v1 = FIELD_EX64(tlb->tlb_entry1, TLBENTRY, V);
    uint64_t tlb_vppn = FIELD_EX64(tlb->tlb_misc, TLB_MISC, VPPN);

    if (index > env->stlb_size) {
        tlb_ps = FIELD_EX64(tlb->tlb_misc, TLB_MISC, PS);
    } else {
        tlb_ps = FIELD_EX64(env->CSR_STLBPS, CSR_STLBPS, PS);
    }
    pagesize = 1 << tlb_ps;
    mask = MAKE_64BIT_MASK(0, tlb_ps + 1);

    if (tlb_v0) {
        addr = (tlb_vppn << R_TLB_MISC_VPPN_SHIFT) & ~mask;    /* even */
        tlb_flush_range_by_mmuidx(env_cpu(env), addr, pagesize,
                                  mmu_idx, TARGET_LONG_BITS);
    }

    if (tlb_v1) {
        addr = (tlb_vppn << R_TLB_MISC_VPPN_SHIFT) & pagesize;    /* odd */
        tlb_flush_range_by_mmuidx(env_cpu(env), addr, pagesize,
                                  mmu_idx, TARGET_LONG_BITS);
    }
}

static void loongarch_invalidate_tlb(CPULoongArchState *env, int index)
{
    loongarch_tlb *tlb;
    uint16_t csr_asid, tlb_asid, tlb_g;

    csr_asid = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);
    tlb = &env->tlb[index];
    tlb_asid = FIELD_EX64(tlb->tlb_misc, TLB_MISC, ASID);
    tlb_g = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, G);
    if (tlb_g == 0 && tlb_asid != csr_asid) {
        return;
    }
    loongarch_invalidate_tlb_entry(env, index);
}

static void loongarch_fill_tlb_entry(CPULoongArchState *env, int index)
{
    loongarch_tlb *tlb = &env->tlb[index];
    uint64_t lo0, lo1, csr_vppn;
    uint16_t csr_asid;
    uint8_t csr_ps;

    if (FIELD_EX64(env->CSR_TLBRERA, CSR_TLBRERA, ISTLBR)) {
        csr_ps = FIELD_EX64(env->CSR_TLBREHI, CSR_TLBREHI, PS);
        csr_vppn = FIELD_EX64(env->CSR_TLBREHI, CSR_TLBREHI, VPPN);
        lo0 = env->CSR_TLBRELO0;
        lo1 = env->CSR_TLBRELO1;
    } else {
        csr_ps = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, PS);
        csr_vppn = FIELD_EX64(env->CSR_TLBEHI, CSR_TLBEHI, VPPN);
        lo0 = env->CSR_TLBELO0;
        lo1 = env->CSR_TLBELO1;
    }

    if (csr_ps == 0) {
        qemu_log_mask(CPU_LOG_MMU, "page size is 0\n");
    }

    /* Only mtlb has the ps fields */
    if (index > env->stlb_size) {
        tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, PS, csr_ps);
    }

    tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, VPPN, csr_vppn);
    tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, E, 1);
    csr_asid = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);
    tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, ASID, csr_asid);

    tlb->tlb_entry0 = lo0;
    tlb->tlb_entry1 = lo1;
}

/* Return random value in [low, high] */
static uint32_t cpu_loongarch_get_random_loongarch_tlb(uint32_t low,
                                                       uint32_t high)
{
    uint32_t val;

    qemu_guest_getrandom_nofail(&val, sizeof(val));
    return val % (high - low + 1) + low;
}

void helper_tlbsrch(CPULoongArchState *env)
{
    int index, match;

    match = loongarch_tlb_search(env, env->CSR_TLBEHI, &index);
    if (match) {
        env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX, INDEX, index);
        env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX, NE, 0);
        return;
    }

    env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX, NE, 1);
}

void helper_tlbrd(CPULoongArchState *env)
{
    loongarch_tlb *tlb;
    int index;
    uint8_t tlb_ps, tlb_e;

    index = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, INDEX);
    tlb = &env->tlb[index];

    if (index > env->stlb_size) {
        tlb_ps = FIELD_EX64(tlb->tlb_misc, TLB_MISC, PS);
    } else {
        tlb_ps = FIELD_EX64(env->CSR_STLBPS, CSR_STLBPS, PS);
    }
    tlb_e = FIELD_EX64(tlb->tlb_misc, TLB_MISC, E);

    if (!tlb_e) {
        /* Invalid TLB entry */
        env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX, NE, 1);
        env->CSR_ASID  = FIELD_DP64(env->CSR_ASID, CSR_ASID, ASID, 0);
        env->CSR_TLBEHI = 0;
        env->CSR_TLBELO0 = 0;
        env->CSR_TLBELO1 = 0;
        env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX, PS, 0);
    } else {
        /* Valid TLB entry */
        env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX, NE, 0);
        env->CSR_TLBIDX = FIELD_DP64(env->CSR_TLBIDX, CSR_TLBIDX,
                                     PS, (tlb_ps & 0x3f));
        env->CSR_TLBEHI = FIELD_EX64(tlb->tlb_misc, TLB_MISC, VPPN) <<
                                     R_TLB_MISC_VPPN_SHIFT;
        env->CSR_TLBELO0 = tlb->tlb_entry0;
        env->CSR_TLBELO1 = tlb->tlb_entry1;
    }
}

void helper_tlbwr(CPULoongArchState *env)
{
    int index = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, INDEX);

    loongarch_invalidate_tlb(env, index);

    if (FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, NE)) {
        env->tlb[index].tlb_misc = FIELD_DP64(env->tlb[index].tlb_misc,
                                              TLB_MISC, E, 0);
        return;
    }

    loongarch_fill_tlb_entry(env, index);
}

void helper_tlbfill(CPULoongArchState *env)
{
    uint64_t address, entryhi;
    int index, set, stlb_idx;
    uint16_t pagesize, stlb_ps;

    if (FIELD_EX64(env->CSR_TLBRERA, CSR_TLBRERA, ISTLBR)) {
        entryhi = env->CSR_TLBREHI;
        pagesize = FIELD_EX64(env->CSR_TLBREHI, CSR_TLBREHI, PS);
    } else {
        entryhi = env->CSR_TLBEHI;
        pagesize = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, PS);
    }

    uint32_t stlb_size = env->stlb_size;
    uint32_t mtlb_size = env->mtlb_size;

    stlb_ps = FIELD_EX64(env->CSR_STLBPS, CSR_STLBPS, PS);

    if (pagesize == stlb_ps && env->stlb_size) {
        /* Only write into STLB bit [47:13]*/
        address = entryhi & ~MAKE_64BIT_MASK(0, R_CSR_TLBEHI_VPPN_SHIFT);

        /* Choose one set ramdomly */
        set = cpu_loongarch_get_random_loongarch_tlb(0, 7);

        /* Index in one set */
        stlb_idx = (address >> (stlb_ps + 1)) & 0xff; /* [0,255] */

        index = set * 256 + stlb_idx;
    } else {
        /* Only write into MTLB */
        index = cpu_loongarch_get_random_loongarch_tlb(
                stlb_size, stlb_size + mtlb_size - 1);
    }

    loongarch_invalidate_tlb(env, index);
    loongarch_fill_tlb_entry(env, index);
}

void helper_tlbclr(CPULoongArchState *env)
{
    loongarch_tlb *tlb;
    int i;
    uint16_t csr_asid, tlb_asid, tlb_g;
    int msize, ssize, index;

    csr_asid = FIELD_EX64(env->CSR_ASID, CSR_ASID, ASID);
    msize = env->mtlb_size;
    ssize = env->stlb_size;
    index = FIELD_EX64(env->CSR_TLBIDX, CSR_TLBIDX, INDEX);

    if (index < ssize) {
        /* STLB. One line per operation */
        for (i = 0; i < 8; i++) {
            tlb = &env->tlb[i * 256 + (index % 256)];
            tlb_asid = FIELD_EX64(tlb->tlb_misc, TLB_MISC, ASID);
            tlb_g = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, G);
            if (!tlb_g && tlb_asid == csr_asid) {
                tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, E, 0);
            }
        }
    } else if (index < (ssize + msize)) {
        /* MTLB. All entries */
        for (i = ssize; i < ssize + msize; i++) {
            tlb = &env->tlb[i];
            tlb_asid = FIELD_EX64(tlb->tlb_misc, TLB_MISC, ASID);
            tlb_g = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, G);
            if (!tlb_g && tlb_asid == csr_asid) {
                tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, E, 0);
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
            int index = i * 256 + (index % 256);
            env->tlb[index].tlb_misc = FIELD_DP64(env->tlb[index].tlb_misc,
                                                 TLB_MISC, E, 0);
        }
    } else if (index < (ssize + msize)) {
        /* MTLB. All entries */
        for (i = ssize; i < ssize + msize; i++) {
            env->tlb[i].tlb_misc = FIELD_DP64(env->tlb[i].tlb_misc,
                                              TLB_MISC, E, 0);
        }
    }

    cpu_loongarch_tlb_flush(env);
}

void helper_invtlb_all(CPULoongArchState *env)
{
    for (int i = 0; i < LOONGARCH_TLB_MAX; i++) {
        env->tlb[i].tlb_misc = FIELD_DP64(env->tlb[i].tlb_misc,
                                          TLB_MISC, E, 0);
    }
    cpu_loongarch_tlb_flush(env);
}

void helper_invtlb_all_g(CPULoongArchState *env, uint32_t g)
{
    for (int i = 0; i < LOONGARCH_TLB_MAX; i++) {
        loongarch_tlb *tlb = &env->tlb[i];
        uint8_t tlb_g = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, G);

        if (tlb_g == g) {
            tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, E, 0);
        }
    }
    cpu_loongarch_tlb_flush(env);
}

void helper_invtlb_all_asid(CPULoongArchState *env, target_ulong info)
{
    uint16_t asid = info & 0x3ff;

    for (int i = 0; i < LOONGARCH_TLB_MAX; i++) {
        loongarch_tlb *tlb = &env->tlb[i];
        uint8_t tlb_g = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, G);
        uint16_t tlb_asid = FIELD_EX64(tlb->tlb_misc, TLB_MISC, ASID);

        if (!tlb_g && (tlb_asid == asid)) {
            tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, E, 0);
        }
    }
    cpu_loongarch_tlb_flush(env);
}

void helper_invtlb_page_asid(CPULoongArchState *env, target_ulong info,
                             target_ulong addr)
{
    uint16_t asid = info & 0x3ff;

    for (int i = 0; i < LOONGARCH_TLB_MAX; i++) {
        loongarch_tlb *tlb = &env->tlb[i];
        uint8_t tlb_g = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, G);
        uint16_t tlb_asid = FIELD_EX64(tlb->tlb_misc, TLB_MISC, ASID);
        uint64_t vpn, tlb_vppn;
        uint8_t tlb_ps, compare_shift;

        if (i > env->stlb_size) {
            tlb_ps = FIELD_EX64(tlb->tlb_misc, TLB_MISC, PS);
        } else {
            tlb_ps = FIELD_EX64(env->CSR_STLBPS, CSR_STLBPS, PS);
        }
        tlb_vppn = FIELD_EX64(tlb->tlb_misc, TLB_MISC, VPPN);
        vpn = (addr & TARGET_VIRT_MASK) >> (tlb_ps + 1);
        compare_shift = tlb_ps + 1 - R_TLB_MISC_VPPN_SHIFT;

        if (!tlb_g && (tlb_asid == asid) &&
           (vpn == (tlb_vppn >> compare_shift))) {
            tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, E, 0);
        }
    }
    cpu_loongarch_tlb_flush(env);
}

void helper_invtlb_page_asid_or_g(CPULoongArchState *env,
                                  target_ulong info, target_ulong addr)
{
    uint16_t asid = info & 0x3ff;

    for (int i = 0; i < LOONGARCH_TLB_MAX; i++) {
        loongarch_tlb *tlb = &env->tlb[i];
        uint8_t tlb_g = FIELD_EX64(tlb->tlb_entry0, TLBENTRY, G);
        uint16_t tlb_asid = FIELD_EX64(tlb->tlb_misc, TLB_MISC, ASID);
        uint64_t vpn, tlb_vppn;
        uint8_t tlb_ps, compare_shift;

        if (i > env->stlb_size) {
            tlb_ps = FIELD_EX64(tlb->tlb_misc, TLB_MISC, PS);
        } else {
            tlb_ps = FIELD_EX64(env->CSR_STLBPS, CSR_STLBPS, PS);
        }
        tlb_vppn = FIELD_EX64(tlb->tlb_misc, TLB_MISC, VPPN);
        vpn = (addr & TARGET_VIRT_MASK) >> (tlb_ps + 1);
        compare_shift = tlb_ps + 1 - R_TLB_MISC_VPPN_SHIFT;

        if ((tlb_g || (tlb_asid == asid)) &&
            (vpn == (tlb_vppn >> compare_shift))) {
            tlb->tlb_misc = FIELD_DP64(tlb->tlb_misc, TLB_MISC, E, 0);
        }
    }
    cpu_loongarch_tlb_flush(env);
}

void loongarch_mmu_init(CPULoongArchState *env)
{
    /* Number of MTLB */
    env->mtlb_size = 64;

    /* Number of STLB */
    env->stlb_size = 2048;

    /* For 16KB, ps = 14, compare the bit [47:15] */
    for (int i = 0; i < LOONGARCH_TLB_MAX; i++) {
        env->tlb[i].tlb_misc = FIELD_DP64(env->tlb[i].tlb_misc, TLB_MISC, E, 0);
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

    /* Data access */
    /* XXX: put correct access by using cpu_restore_state() correctly */
    ret = get_physical_address(env, &physical, &prot, address,
                               access_type, mmu_idx);

    if (ret == TLBRET_MATCH) {
        tlb_set_page(cs, address & TARGET_PAGE_MASK,
                     physical & TARGET_PAGE_MASK, prot,
                     mmu_idx, TARGET_PAGE_SIZE);
        qemu_log_mask(CPU_LOG_MMU,
                      "%s address=%" VADDR_PRIx " physical " TARGET_FMT_plx
                      " prot %d\n", __func__, address, physical, prot);
        return true;
    } else {
        qemu_log_mask(CPU_LOG_MMU,
                      "%s address=%" VADDR_PRIx " ret %d\n", __func__, address,
                      ret);
    }
    if (probe) {
        return false;
    } else {
        raise_mmu_exception(env, address, access_type, ret);
        do_raise_exception(env, cs->exception_index, retaddr);
    }
}
