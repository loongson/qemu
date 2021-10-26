/*
 * LoongArch translate functions for system mode
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

/* Privileged instruction translation */

#include "cpu-csr.h"

#ifdef CONFIG_USER_ONLY

#define GEN_FALSE_TRANS(name)   \
static bool trans_##name(DisasContext *ctx, arg_##name * a)  \
{   \
    return false;   \
}

GEN_FALSE_TRANS(csrxchg)
GEN_FALSE_TRANS(iocsrrd_b)
GEN_FALSE_TRANS(iocsrrd_h)
GEN_FALSE_TRANS(iocsrrd_w)
GEN_FALSE_TRANS(iocsrrd_d)
GEN_FALSE_TRANS(iocsrwr_b)
GEN_FALSE_TRANS(iocsrwr_h)
GEN_FALSE_TRANS(iocsrwr_w)
GEN_FALSE_TRANS(iocsrwr_d)

#else
static bool trans_csrrd(DisasContext *ctx, unsigned rd, unsigned csr)
{
    TCGv dest = gpr_dst(ctx, rd, EXT_NONE);

#define CASE_CSR_RDQ(csr)            \
    case LOONGARCH_CSR_ ## csr:      \
    {                                \
        tcg_gen_ld_tl(dest, cpu_env, offsetof(CPULoongArchState, CSR_##csr)); \
        break;                       \
    };                               \

    switch (csr) {
    CASE_CSR_RDQ(CRMD)
    CASE_CSR_RDQ(PRMD)
    CASE_CSR_RDQ(EUEN)
    CASE_CSR_RDQ(MISC)
    CASE_CSR_RDQ(ECFG)
    CASE_CSR_RDQ(ESTAT)
    CASE_CSR_RDQ(ERA)
    CASE_CSR_RDQ(BADV)
    CASE_CSR_RDQ(BADI)
    CASE_CSR_RDQ(EENTRY)
    CASE_CSR_RDQ(TLBIDX)
    CASE_CSR_RDQ(TLBEHI)
    CASE_CSR_RDQ(TLBELO0)
    CASE_CSR_RDQ(TLBELO1)
    CASE_CSR_RDQ(ASID)
    CASE_CSR_RDQ(PGDL)
    CASE_CSR_RDQ(PGDH)
    CASE_CSR_RDQ(PWCL)
    CASE_CSR_RDQ(PWCH)
    CASE_CSR_RDQ(STLBPS)
    CASE_CSR_RDQ(RVACFG)
    CASE_CSR_RDQ(CPUID)
    CASE_CSR_RDQ(PRCFG1)
    CASE_CSR_RDQ(PRCFG2)
    CASE_CSR_RDQ(PRCFG3)
    CASE_CSR_RDQ(SAVE0)
    CASE_CSR_RDQ(SAVE1)
    CASE_CSR_RDQ(SAVE2)
    CASE_CSR_RDQ(SAVE3)
    CASE_CSR_RDQ(SAVE4)
    CASE_CSR_RDQ(SAVE5)
    CASE_CSR_RDQ(SAVE6)
    CASE_CSR_RDQ(SAVE7)
    CASE_CSR_RDQ(CNTC)
    CASE_CSR_RDQ(LLBCTL)
    CASE_CSR_RDQ(IMPCTL1)
    CASE_CSR_RDQ(IMPCTL2)
    CASE_CSR_RDQ(TLBRENTRY)
    CASE_CSR_RDQ(TLBRBADV)
    CASE_CSR_RDQ(TLBRERA)
    CASE_CSR_RDQ(TLBRSAVE)
    CASE_CSR_RDQ(TLBRELO0)
    CASE_CSR_RDQ(TLBRELO1)
    CASE_CSR_RDQ(TLBREHI)
    CASE_CSR_RDQ(TLBRPRMD)
    CASE_CSR_RDQ(MERRCTL)
    CASE_CSR_RDQ(MERRINFO)
    CASE_CSR_RDQ(MERRINFO1)
    CASE_CSR_RDQ(MERRENT)
    CASE_CSR_RDQ(MERRERA)
    CASE_CSR_RDQ(MERRSAVE)
    CASE_CSR_RDQ(CTAG)
    CASE_CSR_RDQ(DMWIN0)
    CASE_CSR_RDQ(DMWIN1)
    CASE_CSR_RDQ(DMWIN2)
    CASE_CSR_RDQ(DMWIN3)
    CASE_CSR_RDQ(PERFCTRL0)
    CASE_CSR_RDQ(PERFCNTR0)
    CASE_CSR_RDQ(PERFCTRL1)
    CASE_CSR_RDQ(PERFCNTR1)
    CASE_CSR_RDQ(PERFCTRL2)
    CASE_CSR_RDQ(PERFCNTR2)
    CASE_CSR_RDQ(PERFCTRL3)
    CASE_CSR_RDQ(PERFCNTR3)
    /* debug */
    CASE_CSR_RDQ(MWPC)
    CASE_CSR_RDQ(MWPS)
    CASE_CSR_RDQ(DB0ADDR)
    CASE_CSR_RDQ(DB0MASK)
    CASE_CSR_RDQ(DB0CTL)
    CASE_CSR_RDQ(DB0ASID)
    CASE_CSR_RDQ(DB1ADDR)
    CASE_CSR_RDQ(DB1MASK)
    CASE_CSR_RDQ(DB1CTL)
    CASE_CSR_RDQ(DB1ASID)
    CASE_CSR_RDQ(DB2ADDR)
    CASE_CSR_RDQ(DB2MASK)
    CASE_CSR_RDQ(DB2CTL)
    CASE_CSR_RDQ(DB2ASID)
    CASE_CSR_RDQ(DB3ADDR)
    CASE_CSR_RDQ(DB3MASK)
    CASE_CSR_RDQ(DB3CTL)
    CASE_CSR_RDQ(DB3ASID)
    CASE_CSR_RDQ(FWPC)
    CASE_CSR_RDQ(FWPS)
    CASE_CSR_RDQ(IB0ADDR)
    CASE_CSR_RDQ(IB0MASK)
    CASE_CSR_RDQ(IB0CTL)
    CASE_CSR_RDQ(IB0ASID)
    CASE_CSR_RDQ(IB1ADDR)
    CASE_CSR_RDQ(IB1MASK)
    CASE_CSR_RDQ(IB1CTL)
    CASE_CSR_RDQ(IB1ASID)
    CASE_CSR_RDQ(IB2ADDR)
    CASE_CSR_RDQ(IB2MASK)
    CASE_CSR_RDQ(IB2CTL)
    CASE_CSR_RDQ(IB2ASID)
    CASE_CSR_RDQ(IB3ADDR)
    CASE_CSR_RDQ(IB3MASK)
    CASE_CSR_RDQ(IB3CTL)
    CASE_CSR_RDQ(IB3ASID)
    CASE_CSR_RDQ(IB4ADDR)
    CASE_CSR_RDQ(IB4MASK)
    CASE_CSR_RDQ(IB4CTL)
    CASE_CSR_RDQ(IB4ASID)
    CASE_CSR_RDQ(IB5ADDR)
    CASE_CSR_RDQ(IB5MASK)
    CASE_CSR_RDQ(IB5CTL)
    CASE_CSR_RDQ(IB5ASID)
    CASE_CSR_RDQ(IB6ADDR)
    CASE_CSR_RDQ(IB6MASK)
    CASE_CSR_RDQ(IB6CTL)
    CASE_CSR_RDQ(IB6ASID)
    CASE_CSR_RDQ(IB7ADDR)
    CASE_CSR_RDQ(IB7MASK)
    CASE_CSR_RDQ(IB7CTL)
    CASE_CSR_RDQ(IB7ASID)
    CASE_CSR_RDQ(DBG)
    CASE_CSR_RDQ(DERA)
    CASE_CSR_RDQ(DESAVE)
    case LOONGARCH_CSR_PGD:
    case LOONGARCH_CSR_TVAL:
        gen_helper_csr_rdq(dest, cpu_env, tcg_constant_i64(csr));
        break;
    default :
        assert(0);
    }
#undef CASE_CSR_RDQ

    return true;
}

static bool trans_csrwr(DisasContext *ctx, unsigned rd, unsigned csr)
{
    TCGv dest = gpr_dst(ctx, rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, rd, EXT_NONE);
    TCGv temp = tcg_temp_new();

#define CASE_CSR_WRQ(csr)                \
    case LOONGARCH_CSR_ ## csr:          \
    {                                    \
        tcg_gen_ld_tl(temp, cpu_env, offsetof(CPULoongArchState, CSR_##csr)); \
        tcg_gen_st_tl(src1, cpu_env, offsetof(CPULoongArchState, CSR_##csr)); \
        tcg_gen_mov_tl(dest, temp);      \
        tcg_temp_free(temp);             \
        break;                           \
    };                                   \

    switch (csr) {
    case LOONGARCH_CSR_CRMD:
        tcg_gen_ld_tl(temp, cpu_env, offsetof(CPULoongArchState, CSR_CRMD));
        tcg_gen_st_tl(src1, cpu_env, offsetof(CPULoongArchState, CSR_CRMD));
        tcg_gen_mov_tl(dest, temp);
        tcg_gen_movi_tl(cpu_pc, ctx->base.pc_next + 4);
        tcg_temp_free(temp);
        ctx->base.is_jmp = DISAS_EXIT;
        break;
    case LOONGARCH_CSR_EUEN:
        tcg_gen_ld_tl(temp, cpu_env, offsetof(CPULoongArchState, CSR_EUEN));
        tcg_gen_st_tl(src1, cpu_env, offsetof(CPULoongArchState, CSR_EUEN));
        tcg_gen_mov_tl(dest, temp);
        tcg_gen_movi_tl(cpu_pc, ctx->base.pc_next + 4);
        tcg_temp_free(temp);
        ctx->base.is_jmp = DISAS_EXIT;
        break;
    CASE_CSR_WRQ(PRMD)
    CASE_CSR_WRQ(MISC)
    CASE_CSR_WRQ(ECFG)
    CASE_CSR_WRQ(ESTAT)
    CASE_CSR_WRQ(ERA)
    CASE_CSR_WRQ(BADV)
    CASE_CSR_WRQ(BADI)
    CASE_CSR_WRQ(EENTRY)
    CASE_CSR_WRQ(TLBIDX)
    CASE_CSR_WRQ(TLBEHI)
    CASE_CSR_WRQ(TLBELO0)
    CASE_CSR_WRQ(TLBELO1)
    CASE_CSR_WRQ(PGDL)
    CASE_CSR_WRQ(PGDH)
    CASE_CSR_WRQ(PWCL)
    CASE_CSR_WRQ(PWCH)
    CASE_CSR_WRQ(STLBPS)
    CASE_CSR_WRQ(RVACFG)
    CASE_CSR_WRQ(CPUID)
    CASE_CSR_WRQ(PRCFG1)
    CASE_CSR_WRQ(PRCFG2)
    CASE_CSR_WRQ(PRCFG3)
    CASE_CSR_WRQ(SAVE0)
    CASE_CSR_WRQ(SAVE1)
    CASE_CSR_WRQ(SAVE2)
    CASE_CSR_WRQ(SAVE3)
    CASE_CSR_WRQ(SAVE4)
    CASE_CSR_WRQ(SAVE5)
    CASE_CSR_WRQ(SAVE6)
    CASE_CSR_WRQ(SAVE7)
    CASE_CSR_WRQ(TVAL)
    CASE_CSR_WRQ(CNTC)
    CASE_CSR_WRQ(LLBCTL)
    CASE_CSR_WRQ(IMPCTL1)
    CASE_CSR_WRQ(IMPCTL2)
    CASE_CSR_WRQ(TLBRENTRY)
    CASE_CSR_WRQ(TLBRBADV)
    CASE_CSR_WRQ(TLBRERA)
    CASE_CSR_WRQ(TLBRSAVE)
    CASE_CSR_WRQ(TLBRELO0)
    CASE_CSR_WRQ(TLBRELO1)
    CASE_CSR_WRQ(TLBREHI)
    CASE_CSR_WRQ(TLBRPRMD)
    CASE_CSR_WRQ(MERRCTL)
    CASE_CSR_WRQ(MERRINFO)
    CASE_CSR_WRQ(MERRINFO1)
    CASE_CSR_WRQ(MERRENT)
    CASE_CSR_WRQ(MERRERA)
    CASE_CSR_WRQ(MERRSAVE)
    CASE_CSR_WRQ(CTAG)
    CASE_CSR_WRQ(DMWIN0)
    CASE_CSR_WRQ(DMWIN1)
    CASE_CSR_WRQ(DMWIN2)
    CASE_CSR_WRQ(DMWIN3)
    CASE_CSR_WRQ(PERFCTRL0)
    CASE_CSR_WRQ(PERFCNTR0)
    CASE_CSR_WRQ(PERFCTRL1)
    CASE_CSR_WRQ(PERFCNTR1)
    CASE_CSR_WRQ(PERFCTRL2)
    CASE_CSR_WRQ(PERFCNTR2)
    CASE_CSR_WRQ(PERFCTRL3)
    CASE_CSR_WRQ(PERFCNTR3)
    /* debug */
    CASE_CSR_WRQ(MWPC)
    CASE_CSR_WRQ(MWPS)
    CASE_CSR_WRQ(DB0ADDR)
    CASE_CSR_WRQ(DB0MASK)
    CASE_CSR_WRQ(DB0CTL)
    CASE_CSR_WRQ(DB0ASID)
    CASE_CSR_WRQ(DB1ADDR)
    CASE_CSR_WRQ(DB1MASK)
    CASE_CSR_WRQ(DB1CTL)
    CASE_CSR_WRQ(DB1ASID)
    CASE_CSR_WRQ(DB2ADDR)
    CASE_CSR_WRQ(DB2MASK)
    CASE_CSR_WRQ(DB2CTL)
    CASE_CSR_WRQ(DB2ASID)
    CASE_CSR_WRQ(DB3ADDR)
    CASE_CSR_WRQ(DB3MASK)
    CASE_CSR_WRQ(DB3CTL)
    CASE_CSR_WRQ(DB3ASID)
    CASE_CSR_WRQ(FWPC)
    CASE_CSR_WRQ(FWPS)
    CASE_CSR_WRQ(IB0ADDR)
    CASE_CSR_WRQ(IB0MASK)
    CASE_CSR_WRQ(IB0CTL)
    CASE_CSR_WRQ(IB0ASID)
    CASE_CSR_WRQ(IB1ADDR)
    CASE_CSR_WRQ(IB1MASK)
    CASE_CSR_WRQ(IB1CTL)
    CASE_CSR_WRQ(IB1ASID)
    CASE_CSR_WRQ(IB2ADDR)
    CASE_CSR_WRQ(IB2MASK)
    CASE_CSR_WRQ(IB2CTL)
    CASE_CSR_WRQ(IB2ASID)
    CASE_CSR_WRQ(IB3ADDR)
    CASE_CSR_WRQ(IB3MASK)
    CASE_CSR_WRQ(IB3CTL)
    CASE_CSR_WRQ(IB3ASID)
    CASE_CSR_WRQ(IB4ADDR)
    CASE_CSR_WRQ(IB4MASK)
    CASE_CSR_WRQ(IB4CTL)
    CASE_CSR_WRQ(IB4ASID)
    CASE_CSR_WRQ(IB5ADDR)
    CASE_CSR_WRQ(IB5MASK)
    CASE_CSR_WRQ(IB5CTL)
    CASE_CSR_WRQ(IB5ASID)
    CASE_CSR_WRQ(IB6ADDR)
    CASE_CSR_WRQ(IB6MASK)
    CASE_CSR_WRQ(IB6CTL)
    CASE_CSR_WRQ(IB6ASID)
    CASE_CSR_WRQ(IB7ADDR)
    CASE_CSR_WRQ(IB7MASK)
    CASE_CSR_WRQ(IB7CTL)
    CASE_CSR_WRQ(IB7ASID)
    CASE_CSR_WRQ(DBG)
    CASE_CSR_WRQ(DERA)
    CASE_CSR_WRQ(DESAVE)
    case LOONGARCH_CSR_ASID:
        gen_helper_csr_wrq(dest, cpu_env, src1, tcg_constant_i64(csr));
        break;
    default :
        assert(0);
    }
#undef CASE_CSR_WRQ

    return true;
}

static bool trans_csrxchg(DisasContext *ctx, arg_csrxchg *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rd, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rj, EXT_NONE);

    if (a->rj == 0) {
        return trans_csrrd(ctx, a->rd, a->csr);
    } else if (a->rj == 1) {
        return trans_csrwr(ctx, a->rd, a->csr);
    }

    gen_helper_csr_xchgq(dest, cpu_env, src1, src2, tcg_constant_i64(a->csr));

    return true;
}

static bool trans_iocsrrd_b(DisasContext *ctx, arg_iocsrrd_b *a)
{
    TCGv tmp = tcg_temp_new();
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);

    gen_helper_iocsr_read(tmp, cpu_env, src1);
    tcg_gen_qemu_ld_tl(dest, tmp, ctx->mem_idx, MO_SB);
    return true;
}

static bool trans_iocsrrd_h(DisasContext *ctx, arg_iocsrrd_h *a)
{
    TCGv tmp = tcg_temp_new();
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);

    gen_helper_iocsr_read(tmp, cpu_env, src1);
    tcg_gen_qemu_ld_tl(dest, tmp, ctx->mem_idx, MO_TESW);
    return true;
}

static bool trans_iocsrrd_w(DisasContext *ctx, arg_iocsrrd_w *a)
{
    TCGv tmp = tcg_temp_new();
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);

    gen_helper_iocsr_read(tmp, cpu_env, src1);
    tcg_gen_qemu_ld_tl(dest, tmp, ctx->mem_idx, MO_TESL);
    return true;
}

static bool trans_iocsrrd_d(DisasContext *ctx, arg_iocsrrd_d *a)
{
    TCGv tmp = tcg_temp_new();
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);

    gen_helper_iocsr_read(tmp, cpu_env, src1);
    tcg_gen_qemu_ld_tl(dest, tmp, ctx->mem_idx, MO_TEQ);
    return true;
}

static bool trans_iocsrwr_b(DisasContext *ctx, arg_iocsrwr_b *a)
{
    uint32_t oi = make_memop_idx(MO_SB, ctx->mem_idx);
    TCGv val = gpr_src(ctx, a->rd, EXT_NONE);
    TCGv addr = gpr_src(ctx, a->rj, EXT_NONE);

    gen_helper_iocsr_write(cpu_env, addr, val, tcg_constant_i32(oi));
    return true;
}

static bool trans_iocsrwr_h(DisasContext *ctx, arg_iocsrwr_h *a)
{
    uint32_t oi = make_memop_idx(MO_TESW, ctx->mem_idx);
    TCGv val = gpr_src(ctx, a->rd, EXT_NONE);
    TCGv addr = gpr_src(ctx, a->rj, EXT_NONE);

    gen_helper_iocsr_write(cpu_env, addr, val, tcg_constant_i32(oi));
    return true;
}

static bool trans_iocsrwr_w(DisasContext *ctx, arg_iocsrwr_w *a)
{
    uint32_t oi = make_memop_idx(MO_TESL, ctx->mem_idx);
    TCGv val = gpr_src(ctx, a->rd, EXT_NONE);
    TCGv addr = gpr_src(ctx, a->rj, EXT_NONE);

    gen_helper_iocsr_write(cpu_env, addr, val, tcg_constant_i32(oi));
    return true;
}

static bool trans_iocsrwr_d(DisasContext *ctx, arg_iocsrwr_d *a)
{
    uint32_t oi = make_memop_idx(MO_TEQ, ctx->mem_idx);
    TCGv val = gpr_src(ctx, a->rd, EXT_NONE);
    TCGv addr = gpr_src(ctx, a->rj, EXT_NONE);

    gen_helper_iocsr_write(cpu_env, addr, val, tcg_constant_i32(oi));
    return true;
}
#endif
