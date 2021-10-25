/*
 * LoongArch translate functions
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

static bool gen_fload_imm(DisasContext *ctx, arg_fmt_fdrjsi12 *a,
                          MemOp mop, bool nanbox)
{
    TCGv addr = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv temp = NULL;

    if (a->si12) {
        temp = tcg_temp_new();
        tcg_gen_addi_tl(temp, addr, a->si12);
        addr = temp;
    }

    tcg_gen_qemu_ld_tl(cpu_fpr[a->fd], addr, ctx->mem_idx, mop);

    if (nanbox) {
        gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    }

    if (temp) {
        tcg_temp_free(temp);
    }
    return true;
}

static bool gen_fstore_imm(DisasContext *ctx, arg_fmt_fdrjsi12 *a,
                           MemOp mop, bool nanbox)
{
    TCGv addr = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv temp = NULL;

    if (a->si12) {
        temp = tcg_temp_new();
        tcg_gen_addi_tl(temp, addr, a->si12);
        addr = temp;
    }

    if (nanbox) {
        gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    }

    tcg_gen_qemu_st_tl(cpu_fpr[a->fd], addr, ctx->mem_idx, mop);

    if (temp) {
        tcg_temp_free(temp);
    }
    return true;
}

static bool gen_fload_tl(DisasContext *ctx, arg_fmt_fdrjrk *a,
                         MemOp mop, bool nanbox)
{
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rk, EXT_NONE);
    TCGv addr = tcg_temp_new();

    tcg_gen_add_tl(addr, src1, src2);
    tcg_gen_qemu_ld_tl(cpu_fpr[a->fd], addr, ctx->mem_idx, mop);

    if (nanbox) {
        gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    }

    tcg_temp_free(addr);
    return true;
}

static bool gen_fstore_tl(DisasContext *ctx, arg_fmt_fdrjrk *a,
                          MemOp mop, bool nanbox)
{
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rk, EXT_NONE);
    TCGv addr = tcg_temp_new();

    tcg_gen_add_tl(addr, src1, src2);

    if (nanbox) {
        gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    }

    tcg_gen_qemu_st_tl(cpu_fpr[a->fd], addr, ctx->mem_idx, mop);

    tcg_temp_free(addr);
    return true;
}

static bool gen_fload_gt(DisasContext *ctx, arg_fmt_fdrjrk *a,
                         MemOp mop, bool nanbox)
{
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rk, EXT_NONE);
    TCGv addr = tcg_temp_new();

    gen_helper_asrtgt_d(cpu_env, src1, src2);
    tcg_gen_add_tl(addr, src1, src2);
    tcg_gen_qemu_ld_tl(cpu_fpr[a->fd], addr, ctx->mem_idx, mop);

    if (nanbox) {
        gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    }

    tcg_temp_free(addr);
    return true;
}

static bool gen_fstore_gt(DisasContext *ctx, arg_fmt_fdrjrk *a,
                          MemOp mop, bool nanbox)
{
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rk, EXT_NONE);
    TCGv addr = tcg_temp_new();

    gen_helper_asrtgt_d(cpu_env, src1, src2);
    tcg_gen_add_tl(addr, src1, src2);

    if (nanbox) {
        gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    }

    tcg_gen_qemu_st_tl(cpu_fpr[a->fd], addr, ctx->mem_idx, mop);

    tcg_temp_free(addr);
    return true;
}

static bool gen_fload_le(DisasContext *ctx, arg_fmt_fdrjrk *a,
                         MemOp mop, bool nanbox)
{
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rk, EXT_NONE);
    TCGv addr = tcg_temp_new();

    gen_helper_asrtle_d(cpu_env, src1, src2);
    tcg_gen_add_tl(addr, src1, src2);
    tcg_gen_qemu_ld_tl(cpu_fpr[a->fd], addr, ctx->mem_idx, mop);

    if (nanbox) {
        gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    }

    tcg_temp_free(addr);
    return true;
}

static bool gen_fstore_le(DisasContext *ctx, arg_fmt_fdrjrk *a,
                          MemOp mop, bool nanbox)
{
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rk, EXT_NONE);
    TCGv addr = tcg_temp_new();

    gen_helper_asrtle_d(cpu_env, src1, src2);
    tcg_gen_add_tl(addr, src1, src2);

    if (nanbox) {
        gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    }

    tcg_gen_qemu_st_tl(cpu_fpr[a->fd], addr, ctx->mem_idx, mop);

    tcg_temp_free(addr);
    return true;
}

TRANS(fld_s, gen_fload_imm, MO_TESL, true)
TRANS(fst_s, gen_fstore_imm, MO_TEUL, true)
TRANS(fld_d, gen_fload_imm, MO_TEQ, false)
TRANS(fst_d, gen_fstore_imm, MO_TEQ, false)
TRANS(fldx_s, gen_fload_tl, MO_TESL, true)
TRANS(fldx_d, gen_fload_tl, MO_TEQ, false)
TRANS(fstx_s, gen_fstore_tl, MO_TEUL, true)
TRANS(fstx_d, gen_fstore_tl, MO_TEQ, false)
TRANS(fldgt_s, gen_fload_gt, MO_TESL, true)
TRANS(fldgt_d, gen_fload_gt, MO_TEQ, false)
TRANS(fldle_s, gen_fload_le, MO_TESL, true)
TRANS(fldle_d, gen_fload_le, MO_TEQ, false)
TRANS(fstgt_s, gen_fstore_gt, MO_TEUL, true)
TRANS(fstgt_d, gen_fstore_gt, MO_TEQ, false)
TRANS(fstle_s, gen_fstore_le, MO_TEUL, true)
TRANS(fstle_d, gen_fstore_le, MO_TEQ, false)
