/*
 * LoongArch translate functions
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

static bool gen_r2_ui5(DisasContext *ctx, arg_slli_w *a,
                       void(*func)(TCGv, TCGv, TCGv))
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_SIGN);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_ZERO);
    TCGv src2 = tcg_constant_tl(a->ui5);

    func(dest, src1, src2);
    gen_set_gpr(a->rd, dest, EXT_SIGN);

    return true;
}

static bool gen_r2_ui6(DisasContext *ctx, arg_slli_d *a,
                       void(*func)(TCGv, TCGv, target_long))
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);

    func(dest, src1, a->ui6);

    return true;
}

static void gen_sll_w(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv t0 = tcg_temp_new();
    tcg_gen_andi_tl(t0, src2, 0x1f);
    tcg_gen_shl_tl(dest, src1, t0);
    tcg_temp_free(t0);
}

static void gen_srl_w(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv t0 = tcg_temp_new();
    tcg_gen_andi_tl(t0, src2, 0x1f);
    tcg_gen_shr_tl(dest, src1, t0);
    tcg_temp_free(t0);
}

static void gen_sra_w(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv t0 = tcg_temp_new();
    tcg_gen_andi_tl(t0, src2, 0x1f);
    tcg_gen_sar_tl(dest, src1, t0);
    tcg_temp_free(t0);
}

static void gen_sll_d(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv t0 = tcg_temp_new();
    tcg_gen_andi_tl(t0, src2, 0x3f);
    tcg_gen_shl_tl(dest, src1, t0);
    tcg_temp_free(t0);
}

static void gen_srl_d(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv t0 = tcg_temp_new();
    tcg_gen_andi_tl(t0, src2, 0x3f);
    tcg_gen_shr_tl(dest, src1, t0);
    tcg_temp_free(t0);
}

static void gen_sra_d(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv t0 = tcg_temp_new();
    tcg_gen_andi_tl(t0, src2, 0x3f);
    tcg_gen_sar_tl(dest, src1, t0);
    tcg_temp_free(t0);
}

static void gen_rotr_w(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv_i32 t1 = tcg_temp_new_i32();
    TCGv_i32 t2 = tcg_temp_new_i32();
    TCGv t0 = tcg_temp_new();

    tcg_gen_andi_tl(t0, src2, 0x1f);

    tcg_gen_trunc_tl_i32(t1, src1);
    tcg_gen_trunc_tl_i32(t2, t0);

    tcg_gen_rotr_i32(t1, t1, t2);
    tcg_gen_ext_i32_tl(dest, t1);

    tcg_temp_free_i32(t1);
    tcg_temp_free_i32(t2);
    tcg_temp_free(t0);
}

static void gen_rotr_d(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv t0 = tcg_temp_new();
    tcg_gen_andi_tl(t0, src2, 0x3f);
    tcg_gen_rotr_tl(dest, src1, t0);
    tcg_temp_free(t0);
}

static bool trans_srai_w(DisasContext *ctx, arg_srai_w *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_ZERO);

    tcg_gen_sextract_tl(dest, src1, a->ui5, 32 - a->ui5);
    return true;
}

TRANS(sll_w, gen_r3, EXT_ZERO, EXT_NONE, EXT_SIGN, gen_sll_w)
TRANS(srl_w, gen_r3, EXT_ZERO, EXT_NONE, EXT_SIGN, gen_srl_w)
TRANS(sra_w, gen_r3, EXT_SIGN, EXT_NONE, EXT_SIGN, gen_sra_w)
TRANS(sll_d, gen_r3, EXT_NONE, EXT_NONE, EXT_NONE, gen_sll_d)
TRANS(srl_d, gen_r3, EXT_NONE, EXT_NONE, EXT_NONE, gen_srl_d)
TRANS(sra_d, gen_r3, EXT_NONE, EXT_NONE, EXT_NONE, gen_sra_d)
TRANS(rotr_w, gen_r3, EXT_ZERO, EXT_NONE, EXT_SIGN, gen_rotr_w)
TRANS(rotr_d, gen_r3, EXT_NONE, EXT_NONE, EXT_NONE, gen_rotr_d)
TRANS(slli_w, gen_r2_ui5, tcg_gen_shl_tl)
TRANS(slli_d, gen_r2_ui6, tcg_gen_shli_tl)
TRANS(srli_w, gen_r2_ui5, tcg_gen_shr_tl)
TRANS(srli_d, gen_r2_ui6, tcg_gen_shri_tl)
TRANS(srai_d, gen_r2_ui6, tcg_gen_sari_tl)
TRANS(rotri_w, gen_r2_ui5, gen_rotr_w)
TRANS(rotri_d, gen_r2_ui6, tcg_gen_rotri_tl)
