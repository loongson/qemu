/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * LoongArch translate functions for system mode
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 */

/* Privileged instruction translation */

#include "cpu-csr.h"

#ifdef CONFIG_USER_ONLY

#define GEN_FALSE_TRANS(name)   \
static bool trans_##name(DisasContext *ctx, arg_##name * a)  \
{   \
    return false;   \
}

GEN_FALSE_TRANS(csrrd)
GEN_FALSE_TRANS(csrwr)
GEN_FALSE_TRANS(csrxchg)

#else

static bool check_plv(DisasContext *ctx)
{
    if (ctx->base.tb->flags == MMU_USER_IDX) {
        generate_exception(ctx, EXCCODE_IPE);
        return true;
    }
    return false;
}

static bool ro_csr(int csr_num)
{
    /*
     * For now qemu does not support any features of the MISC
     * bits yet treat as a RO CSR.
     */
    if ((csr_num == LOONGARCH_CSR_BADI) || (csr_num == LOONGARCH_CSR_CPUID) ||
        (csr_num == LOONGARCH_CSR_PRCFG1) || (csr_num == LOONGARCH_CSR_PRCFG2) ||
        (csr_num == LOONGARCH_CSR_PRCFG3) || (csr_num == LOONGARCH_CSR_PGD) ||
        (csr_num == LOONGARCH_CSR_TVAL) || (csr_num == LOONGARCH_CSR_MISC)) {
        return true;
    }

    return false;
}

static bool trans_csrrd(DisasContext *ctx, arg_csrrd *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);

    if (check_plv(ctx)) {
        return false;
    }

    unsigned csr_offset = cpu_csr_offset(a->csr);
    if (csr_offset == 0) {
        /* CSR is undefined: read as 0 */
        dest = tcg_constant_tl(0);
        return true;
    }

    if ((a->csr == LOONGARCH_CSR_PGD) || (a->csr == LOONGARCH_CSR_CPUID) ||
        (a->csr == LOONGARCH_CSR_TVAL)) {
        gen_helper_csr_rdq(dest, cpu_env, tcg_constant_i64(a->csr));
    } else {
        tcg_gen_ld_tl(dest, cpu_env, csr_offset);
    }
    return true;
}

static bool trans_csrwr(DisasContext *ctx, arg_csrwr *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rd, EXT_NONE);

    if (check_plv(ctx) || ro_csr(a->csr)) {
        return false;
    }

    unsigned csr_offset = cpu_csr_offset(a->csr);
    if (csr_offset == 0) {
        /* CSR is undefined: write ignored. */
        return true;
    }

    if ((a->csr == LOONGARCH_CSR_ASID) || (a->csr == LOONGARCH_CSR_TCFG) ||
        (a->csr == LOONGARCH_CSR_TICLR) || (a->csr == LOONGARCH_CSR_ESTAT)) {
        gen_helper_csr_wrq(dest, cpu_env, src1, tcg_constant_i64(a->csr));
    } else {
        TCGv temp = tcg_temp_new();
        tcg_gen_ld_tl(temp, cpu_env, csr_offset);
        tcg_gen_st_tl(src1, cpu_env, csr_offset);
        tcg_gen_mov_tl(dest, temp);
        tcg_temp_free(temp);

        /* Cpu state may be changed, need exit */
        if ((a->csr == LOONGARCH_CSR_CRMD) || (a->csr == LOONGARCH_CSR_EUEN)) {
            tcg_gen_movi_tl(cpu_pc, ctx->base.pc_next + 4);
            ctx->base.is_jmp = DISAS_EXIT;
        }
    }

    return true;
}

static bool trans_csrxchg(DisasContext *ctx, arg_csrxchg *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rd, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rj, EXT_NONE);

    if (check_plv(ctx) || ro_csr(a->csr)) {
        return false;
    }
    gen_helper_csr_xchgq(dest, cpu_env, src1, src2, tcg_constant_i64(a->csr));
    return true;
}

#endif
