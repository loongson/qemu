/*
 * LoongArch translate functions
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

/* Fixed point arithmetic operation instruction translation */
static bool trans_add_w(DisasContext *ctx, arg_add_w *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (a->rj != 0 && a->rk != 0) {
        tcg_gen_add_tl(Rd, Rj, Rk);
        tcg_gen_ext32s_tl(Rd, Rd);
    } else if (a->rj == 0 && a->rk != 0) {
        tcg_gen_mov_tl(Rd, Rk);
    } else if (a->rj != 0 && a->rk == 0) {
        tcg_gen_mov_tl(Rd, Rj);
    } else {
        tcg_gen_movi_tl(Rd, 0);
    }

    return true;
}

static bool trans_add_d(DisasContext *ctx, arg_add_d *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    check_loongarch_64(ctx);
    if (a->rj != 0 && a->rk != 0) {
        tcg_gen_add_tl(Rd, Rj, Rk);
    } else if (a->rj == 0 && a->rk != 0) {
        tcg_gen_mov_tl(Rd, Rk);
    } else if (a->rj != 0 && a->rk == 0) {
        tcg_gen_mov_tl(Rd, Rj);
    } else {
        tcg_gen_movi_tl(Rd, 0);
    }

    return true;
}

static bool trans_sub_w(DisasContext *ctx, arg_sub_w *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (a->rj != 0 && a->rk != 0) {
        tcg_gen_sub_tl(Rd, Rj, Rk);
        tcg_gen_ext32s_tl(Rd, Rd);
    } else if (a->rj == 0 && a->rk != 0) {
        tcg_gen_neg_tl(Rd, Rk);
        tcg_gen_ext32s_tl(Rd, Rd);
    } else if (a->rj != 0 && a->rk == 0) {
        tcg_gen_mov_tl(Rd, Rj);
    } else {
        tcg_gen_movi_tl(Rd, 0);
    }

    return true;
}

static bool trans_sub_d(DisasContext *ctx, arg_sub_d *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    check_loongarch_64(ctx);
    if (a->rj != 0 && a->rk != 0) {
        tcg_gen_sub_tl(Rd, Rj, Rk);
    } else if (a->rj == 0 && a->rk != 0) {
        tcg_gen_neg_tl(Rd, Rk);
    } else if (a->rj != 0 && a->rk == 0) {
        tcg_gen_mov_tl(Rd, Rj);
    } else {
        tcg_gen_movi_tl(Rd, 0);
    }

    return true;
}

static bool trans_slt(DisasContext *ctx, arg_slt *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);

    tcg_gen_setcond_tl(TCG_COND_LT, Rd, t0, t1);

    return true;
}

static bool trans_sltu(DisasContext *ctx, arg_sltu *a)
{

    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);

    tcg_gen_setcond_tl(TCG_COND_LTU, Rd, t0, t1);

    return true;
}

static bool trans_slti(DisasContext *ctx, arg_slti *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    target_ulong uimm = (target_long)(a->si12);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);

    tcg_gen_setcondi_tl(TCG_COND_LT, Rd, t0, uimm);

    return true;
}

static bool trans_sltui(DisasContext *ctx, arg_sltui *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    target_ulong uimm = (target_long)(a->si12);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);

    tcg_gen_setcondi_tl(TCG_COND_LTU, Rd, t0, uimm);

    return true;
}

static bool trans_nor(DisasContext *ctx, arg_nor *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (a->rj != 0 && a->rk != 0) {
        tcg_gen_nor_tl(Rd, Rj, Rk);
    } else if (a->rj == 0 && a->rk != 0) {
        tcg_gen_not_tl(Rd, Rk);
    } else if (a->rj != 0 && a->rk == 0) {
        tcg_gen_not_tl(Rd, Rj);
    } else {
        tcg_gen_movi_tl(Rd, ~((target_ulong)0));
    }

    return true;
}

static bool trans_and(DisasContext *ctx, arg_and *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (likely(a->rj != 0 && a->rk != 0)) {
        tcg_gen_and_tl(Rd, Rj, Rk);
    } else {
        tcg_gen_movi_tl(Rd, 0);
    }

    return true;
}

static bool trans_or(DisasContext *ctx, arg_or *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (likely(a->rj != 0 && a->rk != 0)) {
        tcg_gen_or_tl(Rd, Rj, Rk);
    } else if (a->rj == 0 && a->rk != 0) {
        tcg_gen_mov_tl(Rd, Rk);
    } else if (a->rj != 0 && a->rk == 0) {
        tcg_gen_mov_tl(Rd, Rj);
    } else {
        tcg_gen_movi_tl(Rd, 0);
    }

    return true;
}

static bool trans_xor(DisasContext *ctx, arg_xor *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (likely(a->rj != 0 && a->rk != 0)) {
        tcg_gen_xor_tl(Rd, Rj, Rk);
    } else if (a->rj == 0 && a->rk != 0) {
        tcg_gen_mov_tl(Rd, Rk);
    } else if (a->rj != 0 && a->rk == 0) {
        tcg_gen_mov_tl(Rd, Rj);
    } else {
        tcg_gen_movi_tl(Rd, 0);
    }

    return true;
}

static bool trans_orn(DisasContext *ctx, arg_orn *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    TCGv t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rk);

    tcg_gen_not_tl(t0, t0);
    tcg_gen_or_tl(Rd, Rj, t0);

    tcg_temp_free(t0);
    return true;
}

static bool trans_andn(DisasContext *ctx, arg_andn *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    TCGv t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rk);

    tcg_gen_not_tl(t0, t0);
    tcg_gen_and_tl(Rd, Rj, t0);

    tcg_temp_free(t0);
    return true;
}

static bool trans_mul_w(DisasContext *ctx, arg_mul_w *a)
{
    TCGv t0, t1;
    TCGv_i32 t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);
    t2 = tcg_temp_new_i32();
    t3 = tcg_temp_new_i32();

    tcg_gen_trunc_tl_i32(t2, t0);
    tcg_gen_trunc_tl_i32(t3, t1);
    tcg_gen_mul_i32(t2, t2, t3);
    tcg_gen_ext_i32_tl(Rd, t2);

    tcg_temp_free_i32(t2);
    tcg_temp_free_i32(t3);

    return true;
}

static bool trans_mulh_w(DisasContext *ctx, arg_mulh_w *a)
{
    TCGv t0, t1;
    TCGv_i32 t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);
    t2 = tcg_temp_new_i32();
    t3 = tcg_temp_new_i32();

    tcg_gen_trunc_tl_i32(t2, t0);
    tcg_gen_trunc_tl_i32(t3, t1);
    tcg_gen_muls2_i32(t2, t3, t2, t3);
    tcg_gen_ext_i32_tl(Rd, t3);

    tcg_temp_free_i32(t2);
    tcg_temp_free_i32(t3);

    return true;
}

static bool trans_mulh_wu(DisasContext *ctx, arg_mulh_wu *a)
{
    TCGv t0, t1;
    TCGv_i32 t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);
    t2 = tcg_temp_new_i32();
    t3 = tcg_temp_new_i32();

    tcg_gen_trunc_tl_i32(t2, t0);
    tcg_gen_trunc_tl_i32(t3, t1);
    tcg_gen_mulu2_i32(t2, t3, t2, t3);
    tcg_gen_ext_i32_tl(Rd, t3);

    tcg_temp_free_i32(t2);
    tcg_temp_free_i32(t3);

    return true;
}

static bool trans_mul_d(DisasContext *ctx, arg_mul_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);

    check_loongarch_64(ctx);
    tcg_gen_mul_i64(Rd, t0, t1);

    return true;
}

static bool trans_mulh_d(DisasContext *ctx, arg_mulh_d *a)
{
    TCGv t0, t1, t2;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);
    t2 = tcg_temp_new();

    check_loongarch_64(ctx);
    tcg_gen_muls2_i64(t2, Rd, t0, t1);

    tcg_temp_free(t2);

    return true;
}

static bool trans_mulh_du(DisasContext *ctx, arg_mulh_du *a)
{
    TCGv t0, t1, t2;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);
    t2 = tcg_temp_new();

    check_loongarch_64(ctx);
    tcg_gen_mulu2_i64(t2, Rd, t0, t1);

    tcg_temp_free(t2);

    return true;
}

static bool trans_mulw_d_w(DisasContext *ctx, arg_mulw_d_w *a)
{
    TCGv_i64 t0, t1, t2;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new_i64();
    t1 = tcg_temp_new_i64();
    t2 = tcg_temp_new_i64();

    gen_load_gpr(t0, a->rj);
    gen_load_gpr(t1, a->rk);

    tcg_gen_ext32s_i64(t0, t0);
    tcg_gen_ext32s_i64(t1, t1);
    tcg_gen_mul_i64(t2, t0, t1);
    tcg_gen_mov_tl(Rd, t2);

    tcg_temp_free_i64(t0);
    tcg_temp_free_i64(t1);
    tcg_temp_free_i64(t2);

    return true;
}

static bool trans_mulw_d_wu(DisasContext *ctx, arg_mulw_d_wu *a)
{
    TCGv_i64 t0, t1, t2;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new_i64();
    t1 = tcg_temp_new_i64();
    t2 = tcg_temp_new_i64();

    gen_load_gpr(t0, a->rj);
    gen_load_gpr(t1, a->rk);

    tcg_gen_ext32u_i64(t0, t0);
    tcg_gen_ext32u_i64(t1, t1);
    tcg_gen_mul_i64(t2, t0, t1);
    tcg_gen_mov_tl(Rd, t2);

    tcg_temp_free_i64(t0);
    tcg_temp_free_i64(t1);
    tcg_temp_free_i64(t2);

    return true;
}

static bool trans_div_w(DisasContext *ctx, arg_div_w *a)
{
    TCGv t0, t1, t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();
    t2 = tcg_temp_new();
    t3 = tcg_temp_new();

    gen_load_gpr(t0, a->rj);
    gen_load_gpr(t1, a->rk);

    tcg_gen_ext32s_tl(t0, t0);
    tcg_gen_ext32s_tl(t1, t1);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t2, t0, INT_MIN);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t3, t1, -1);
    tcg_gen_and_tl(t2, t2, t3);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t3, t1, 0);
    tcg_gen_or_tl(t2, t2, t3);
    tcg_gen_movi_tl(t3, 0);
    tcg_gen_movcond_tl(TCG_COND_NE, t1, t2, t3, t2, t1);
    tcg_gen_div_tl(Rd, t0, t1);
    tcg_gen_ext32s_tl(Rd, Rd);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
    tcg_temp_free(t2);
    tcg_temp_free(t3);

    return true;
}

static bool trans_mod_w(DisasContext *ctx, arg_mod_w *a)
{
    TCGv t0, t1, t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();
    t2 = tcg_temp_new();
    t3 = tcg_temp_new();

    gen_load_gpr(t0, a->rj);
    gen_load_gpr(t1, a->rk);

    tcg_gen_ext32s_tl(t0, t0);
    tcg_gen_ext32s_tl(t1, t1);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t2, t0, INT_MIN);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t3, t1, -1);
    tcg_gen_and_tl(t2, t2, t3);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t3, t1, 0);
    tcg_gen_or_tl(t2, t2, t3);
    tcg_gen_movi_tl(t3, 0);
    tcg_gen_movcond_tl(TCG_COND_NE, t1, t2, t3, t2, t1);
    tcg_gen_rem_tl(Rd, t0, t1);
    tcg_gen_ext32s_tl(Rd, Rd);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
    tcg_temp_free(t2);
    tcg_temp_free(t3);

    return true;
}

static bool trans_div_wu(DisasContext *ctx, arg_div_wu *a)
{
    TCGv t0, t1, t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();
    t2 = tcg_const_tl(0);
    t3 = tcg_const_tl(1);

    gen_load_gpr(t0, a->rj);
    gen_load_gpr(t1, a->rk);

    tcg_gen_ext32u_tl(t0, t0);
    tcg_gen_ext32u_tl(t1, t1);
    tcg_gen_movcond_tl(TCG_COND_EQ, t1, t1, t2, t3, t1);
    tcg_gen_divu_tl(Rd, t0, t1);
    tcg_gen_ext32s_tl(Rd, Rd);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
    tcg_temp_free(t2);
    tcg_temp_free(t3);

    return true;
}

static bool trans_mod_wu(DisasContext *ctx, arg_mod_wu *a)
{
    TCGv t0, t1, t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();
    t2 = tcg_const_tl(0);
    t3 = tcg_const_tl(1);

    gen_load_gpr(t0, a->rj);
    gen_load_gpr(t1, a->rk);

    tcg_gen_ext32u_tl(t0, t0);
    tcg_gen_ext32u_tl(t1, t1);
    tcg_gen_movcond_tl(TCG_COND_EQ, t1, t1, t2, t3, t1);
    tcg_gen_remu_tl(Rd, t0, t1);
    tcg_gen_ext32s_tl(Rd, Rd);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
    tcg_temp_free(t2);
    tcg_temp_free(t3);

    return true;
}

static bool trans_div_d(DisasContext *ctx, arg_div_d *a)
{
    TCGv t0, t1, t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);
    t2 = tcg_temp_new();
    t3 = tcg_temp_new();

    check_loongarch_64(ctx);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t2, t0, -1LL << 63);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t3, t1, -1LL);
    tcg_gen_and_tl(t2, t2, t3);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t3, t1, 0);
    tcg_gen_or_tl(t2, t2, t3);
    tcg_gen_movi_tl(t3, 0);
    tcg_gen_movcond_tl(TCG_COND_NE, t1, t2, t3, t2, t1);
    tcg_gen_div_tl(Rd, t0, t1);

    tcg_temp_free(t2);
    tcg_temp_free(t3);

    return true;
}

static bool trans_mod_d(DisasContext *ctx, arg_mod_d *a)
{
    TCGv t0, t1, t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);
    t2 = tcg_temp_new();
    t3 = tcg_temp_new();

    check_loongarch_64(ctx);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t2, t0, -1LL << 63);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t3, t1, -1LL);
    tcg_gen_and_tl(t2, t2, t3);
    tcg_gen_setcondi_tl(TCG_COND_EQ, t3, t1, 0);
    tcg_gen_or_tl(t2, t2, t3);
    tcg_gen_movi_tl(t3, 0);
    tcg_gen_movcond_tl(TCG_COND_NE, t1, t2, t3, t2, t1);
    tcg_gen_rem_tl(Rd, t0, t1);

    tcg_temp_free(t2);
    tcg_temp_free(t3);

    return true;
}

static bool trans_div_du(DisasContext *ctx, arg_div_du *a)
{
    TCGv t0, t1, t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);
    t2 = tcg_const_tl(0);
    t3 = tcg_const_tl(1);

    check_loongarch_64(ctx);
    tcg_gen_movcond_tl(TCG_COND_EQ, t1, t1, t2, t3, t1);
    tcg_gen_divu_i64(Rd, t0, t1);

    tcg_temp_free(t2);
    tcg_temp_free(t3);

    return true;
}

static bool trans_mod_du(DisasContext *ctx, arg_mod_du *a)
{
    TCGv t0, t1, t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);
    t2 = tcg_const_tl(0);
    t3 = tcg_const_tl(1);

    check_loongarch_64(ctx);
    tcg_gen_movcond_tl(TCG_COND_EQ, t1, t1, t2, t3, t1);
    tcg_gen_remu_i64(Rd, t0, t1);

    tcg_temp_free(t2);
    tcg_temp_free(t3);

    return true;
}

static bool trans_alsl_w(DisasContext *ctx, arg_alsl_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rk);

    gen_load_gpr(t0, a->rj);

    tcg_gen_shli_tl(t0, t0, a->sa2 + 1);
    tcg_gen_add_tl(Rd, t0, t1);
    tcg_gen_ext32s_tl(Rd, Rd);

    tcg_temp_free(t0);

    return true;
}

static bool trans_alsl_wu(DisasContext *ctx, arg_alsl_wu *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rk);

    gen_load_gpr(t0, a->rj);

    tcg_gen_shli_tl(t0, t0, a->sa2 + 1);
    tcg_gen_add_tl(t0, t0, t1);
    tcg_gen_ext32u_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_alsl_d(DisasContext *ctx, arg_alsl_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rk);

    gen_load_gpr(t0, a->rj);

    check_loongarch_64(ctx);
    tcg_gen_shli_tl(t0, t0, a->sa2 + 1);
    tcg_gen_add_tl(Rd, t0, t1);

    tcg_temp_free(t0);

    return true;
}

static bool trans_lu12i_w(DisasContext *ctx, arg_lu12i_w *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    tcg_gen_movi_tl(Rd, a->si20 << 12);

    return true;
}

static bool trans_lu32i_d(DisasContext *ctx, arg_lu32i_d *a)
{
    TCGv_i64 t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new_i64();
    t1 = tcg_temp_new_i64();

    tcg_gen_movi_tl(t0, a->si20);
    tcg_gen_concat_tl_i64(t1, Rd, t0);
    tcg_gen_mov_tl(Rd, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_lu52i_d(DisasContext *ctx, arg_lu52i_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_load_gpr(t1, a->rj);

    tcg_gen_movi_tl(t0, a->si12);
    tcg_gen_shli_tl(t0, t0, 52);
    tcg_gen_andi_tl(t1, t1, 0xfffffffffffffU);
    tcg_gen_or_tl(Rd, t0, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_pcaddi(DisasContext *ctx, arg_pcaddi *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    target_ulong pc = ctx->base.pc_next;
    target_ulong addr = pc + (a->si20 << 2);
    tcg_gen_movi_tl(Rd, addr);

    return true;
}

static bool trans_pcalau12i(DisasContext *ctx, arg_pcalau12i *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    target_ulong pc = ctx->base.pc_next;
    target_ulong addr = (pc + (a->si20 << 12)) & ~0xfff;
    tcg_gen_movi_tl(Rd, addr);

    return true;
}

static bool trans_pcaddu12i(DisasContext *ctx, arg_pcaddu12i *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    target_ulong pc = ctx->base.pc_next;
    target_ulong addr = pc + (a->si20 << 12);
    tcg_gen_movi_tl(Rd, addr);

    return true;
}

static bool trans_pcaddu18i(DisasContext *ctx, arg_pcaddu18i *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    target_ulong pc = ctx->base.pc_next;
    target_ulong addr = pc + ((target_ulong)(a->si20) << 18);
    tcg_gen_movi_tl(Rd, addr);

    return true;
}

static bool trans_addi_w(DisasContext *ctx, arg_addi_w *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    target_ulong uimm = (target_long)(a->si12);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (a->rj != 0) {
        tcg_gen_addi_tl(Rd, Rj, uimm);
        tcg_gen_ext32s_tl(Rd, Rd);
    } else {
        tcg_gen_movi_tl(Rd, uimm);
    }

    return true;
}

static bool trans_addi_d(DisasContext *ctx, arg_addi_d *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    target_ulong uimm = (target_long)(a->si12);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    check_loongarch_64(ctx);
    if (a->rj != 0) {
        tcg_gen_addi_tl(Rd, Rj, uimm);
    } else {
        tcg_gen_movi_tl(Rd, uimm);
    }

    return true;
}

static bool trans_addu16i_d(DisasContext *ctx, arg_addu16i_d *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (a->rj != 0) {
        tcg_gen_addi_tl(Rd, Rj, a->si16 << 16);
    } else {
        tcg_gen_movi_tl(Rd, a->si16 << 16);
    }
    return true;
}

static bool trans_andi(DisasContext *ctx, arg_andi *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];

    target_ulong uimm = (uint16_t)(a->ui12);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (likely(a->rj != 0)) {
        tcg_gen_andi_tl(Rd, Rj, uimm);
    } else {
        tcg_gen_movi_tl(Rd, 0);
    }

    return true;
}

static bool trans_ori(DisasContext *ctx, arg_ori *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];

    target_ulong uimm = (uint16_t)(a->ui12);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (a->rj != 0) {
        tcg_gen_ori_tl(Rd, Rj, uimm);
    } else {
        tcg_gen_movi_tl(Rd, uimm);
    }

    return true;
}

static bool trans_xori(DisasContext *ctx, arg_xori *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];

    target_ulong uimm = (uint16_t)(a->ui12);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (likely(a->rj != 0)) {
        tcg_gen_xori_tl(Rd, Rj, uimm);
    } else {
        tcg_gen_movi_tl(Rd, uimm);
    }

    return true;
}

/* Fixed point shift operation instruction translation */
static bool trans_sll_w(DisasContext *ctx, arg_sll_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rk);

    tcg_gen_andi_tl(t0, t0, 0x1f);
    tcg_gen_shl_tl(t0, t1, t0);
    tcg_gen_ext32s_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_srl_w(DisasContext *ctx, arg_srl_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_load_gpr(t0, a->rk);
    gen_load_gpr(t1, a->rj);

    tcg_gen_ext32u_tl(t1, t1);
    tcg_gen_andi_tl(t0, t0, 0x1f);
    tcg_gen_shr_tl(t0, t1, t0);
    tcg_gen_ext32s_tl(Rd, t0);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_sra_w(DisasContext *ctx, arg_sra_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rk);

    tcg_gen_andi_tl(t0, t0, 0x1f);
    tcg_gen_sar_tl(Rd, t1, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_sll_d(DisasContext *ctx, arg_sll_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rk);

    check_loongarch_64(ctx);
    tcg_gen_andi_tl(t0, t0, 0x3f);
    tcg_gen_shl_tl(Rd, t1, t0);

    tcg_temp_free(t0);

    return true;
}
static bool trans_srl_d(DisasContext *ctx, arg_srl_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rk);

    check_loongarch_64(ctx);
    tcg_gen_andi_tl(t0, t0, 0x3f);
    tcg_gen_shr_tl(Rd, t1, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_sra_d(DisasContext *ctx, arg_sra_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rk);

    check_loongarch_64(ctx);
    tcg_gen_andi_tl(t0, t0, 0x3f);
    tcg_gen_sar_tl(Rd, t1, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_rotr_w(DisasContext *ctx, arg_rotr_w *a)
{
    TCGv t0, t1;
    TCGv_i32 t2, t3;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);
    t2 = tcg_temp_new_i32();
    t3 = tcg_temp_new_i32();

    tcg_gen_trunc_tl_i32(t2, t0);
    tcg_gen_trunc_tl_i32(t3, t1);
    tcg_gen_andi_i32(t2, t2, 0x1f);
    tcg_gen_rotr_i32(t2, t3, t2);
    tcg_gen_ext_i32_tl(Rd, t2);

    tcg_temp_free_i32(t2);
    tcg_temp_free_i32(t3);

    return true;
}

static bool trans_rotr_d(DisasContext *ctx, arg_rotr_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rk);

    check_loongarch_64(ctx);
    tcg_gen_andi_tl(t0, t0, 0x3f);
    tcg_gen_rotr_tl(Rd, t1, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_slli_w(DisasContext *ctx, arg_slli_w *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    TCGv t0 = tcg_temp_new();

    gen_load_gpr(t0, a->rj);
    tcg_gen_shli_tl(t0, t0, a->ui5);
    tcg_gen_ext32s_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_slli_d(DisasContext *ctx, arg_slli_d *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    TCGv t0 = tcg_temp_new();

    gen_load_gpr(t0, a->rj);
    tcg_gen_shli_tl(Rd, t0, a->ui6);

    tcg_temp_free(t0);

    return true;
}

static bool trans_srli_w(DisasContext *ctx, arg_srli_w *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    target_ulong uimm = ((uint16_t)(a->ui5)) & 0x1f;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();

    gen_load_gpr(t0, a->rj);

    if (uimm != 0) {
        tcg_gen_ext32u_tl(t0, t0);
        tcg_gen_shri_tl(Rd, t0, uimm);
    } else {
        tcg_gen_ext32s_tl(Rd, t0);
    }

    tcg_temp_free(t0);

    return true;
}

static bool trans_srli_d(DisasContext *ctx, arg_srli_d *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);

    tcg_gen_shri_tl(Rd, t0, a->ui6);

    return true;
}

static bool trans_srai_w(DisasContext *ctx, arg_srai_w *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    target_ulong uimm = ((uint16_t)(a->ui5)) & 0x1f;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);

    tcg_gen_sari_tl(Rd, t0, uimm);

    return true;
}

static bool trans_srai_d(DisasContext *ctx, arg_srai_d *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);

    check_loongarch_64(ctx);
    tcg_gen_sari_tl(Rd, t0, a->ui6);

    return true;
}

static bool trans_rotri_w(DisasContext *ctx, arg_rotri_w *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    target_ulong uimm = ((uint16_t)(a->ui5)) & 0x1f;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);

    if (uimm != 0) {
        TCGv_i32 t1 = tcg_temp_new_i32();

        tcg_gen_trunc_tl_i32(t1, t0);
        tcg_gen_rotri_i32(t1, t1, uimm);
        tcg_gen_ext_i32_tl(Rd, t1);

        tcg_temp_free_i32(t1);
    } else {
        tcg_gen_ext32s_tl(Rd, t0);
    }

    return true;
}

static bool trans_rotri_d(DisasContext *ctx, arg_rotri_d *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);

    check_loongarch_64(ctx);
    tcg_gen_rotri_tl(Rd, t0, a->ui6);

    return true;
}

/* Fixed point bit operation instruction translation */
static bool trans_ext_w_h(DisasContext *ctx, arg_ext_w_h *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);

    tcg_gen_ext16s_tl(Rd, t0);

    return true;
}

static bool trans_ext_w_b(DisasContext *ctx, arg_ext_w_b *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rj);

    tcg_gen_ext8s_tl(Rd, t0);

    return true;
}

static bool trans_clo_w(DisasContext *ctx, arg_clo_w *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    gen_load_gpr(Rd, a->rj);

    tcg_gen_not_tl(Rd, Rd);
    tcg_gen_ext32u_tl(Rd, Rd);
    tcg_gen_clzi_tl(Rd, Rd, TARGET_LONG_BITS);
    tcg_gen_subi_tl(Rd, Rd, TARGET_LONG_BITS - 32);

    return true;
}

static bool trans_clz_w(DisasContext *ctx, arg_clz_w *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    gen_load_gpr(Rd, a->rj);

    tcg_gen_ext32u_tl(Rd, Rd);
    tcg_gen_clzi_tl(Rd, Rd, TARGET_LONG_BITS);
    tcg_gen_subi_tl(Rd, Rd, TARGET_LONG_BITS - 32);

    return true;
}

static bool trans_cto_w(DisasContext *ctx, arg_cto_w *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rj);

    gen_helper_cto_w(Rd, cpu_env, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_ctz_w(DisasContext *ctx, arg_ctz_w *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rj);

    gen_helper_ctz_w(Rd, cpu_env, t0);

    tcg_temp_free(t0);

    return true;
}
static bool trans_clo_d(DisasContext *ctx, arg_clo_d *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    check_loongarch_64(ctx);
    gen_load_gpr(Rd, a->rj);
    tcg_gen_not_tl(Rd, Rd);
    tcg_gen_clzi_i64(Rd, Rd, 64);

    return true;
}

static bool trans_clz_d(DisasContext *ctx, arg_clz_d *a)
{
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    check_loongarch_64(ctx);
    gen_load_gpr(Rd, a->rj);
    tcg_gen_clzi_i64(Rd, Rd, 64);

    return true;
}

static bool trans_cto_d(DisasContext *ctx, arg_cto_d *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rj);

    gen_helper_cto_d(Rd, cpu_env, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_ctz_d(DisasContext *ctx, arg_ctz_d *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rj);

    gen_helper_ctz_d(Rd, cpu_env, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_revb_2h(DisasContext *ctx, arg_revb_2h *a)
{
    TCGv t0, t1, mask;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();
    mask = tcg_const_tl(0x00FF00FF);

    gen_load_gpr(t0, a->rj);

    tcg_gen_shri_tl(t1, t0, 8);
    tcg_gen_and_tl(t1, t1, mask);
    tcg_gen_and_tl(t0, t0, mask);
    tcg_gen_shli_tl(t0, t0, 8);
    tcg_gen_or_tl(t0, t0, t1);
    tcg_gen_ext32s_tl(Rd, t0);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
    tcg_temp_free(mask);

    return true;
}

static bool trans_revb_4h(DisasContext *ctx, arg_revb_4h *a)
{
    TCGv t0, t1, mask;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();
    mask = tcg_const_tl(0x00FF00FF00FF00FFULL);

    gen_load_gpr(t0, a->rj);

    check_loongarch_64(ctx);
    tcg_gen_shri_tl(t1, t0, 8);
    tcg_gen_and_tl(t1, t1, mask);
    tcg_gen_and_tl(t0, t0, mask);
    tcg_gen_shli_tl(t0, t0, 8);
    tcg_gen_or_tl(Rd, t0, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
    tcg_temp_free(mask);

    return true;
}

static bool trans_revb_2w(DisasContext *ctx, arg_revb_2w *a)
{
    TCGv_i64 t0, t1, t2;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new_i64();
    t1 = tcg_temp_new_i64();
    t2 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rd);

    tcg_gen_ext32u_i64(t1, t2);
    tcg_gen_bswap32_i64(t0, t1);
    tcg_gen_shri_i64(t1, t2, 32);
    tcg_gen_bswap32_i64(t1, t1);
    tcg_gen_concat32_i64(Rd, t0, t1);

    tcg_temp_free_i64(t0);
    tcg_temp_free_i64(t1);

    return true;
}

static bool trans_revb_d(DisasContext *ctx, arg_revb_d *a)
{
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    check_loongarch_64(ctx);
    tcg_gen_bswap64_i64(Rd, Rj);

    return true;
}

static bool trans_revh_2w(DisasContext *ctx, arg_revh_2w *a)
{
    TCGv_i64 t0, t1, t2, mask;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new_i64();
    t1 = tcg_temp_new_i64();
    t2 = get_gpr(a->rj);
    mask = tcg_const_i64(0x0000ffff0000ffffull);

    gen_load_gpr(t1, a->rd);

    tcg_gen_shri_i64(t0, t2, 16);
    tcg_gen_and_i64(t1, t2, mask);
    tcg_gen_and_i64(t0, t0, mask);
    tcg_gen_shli_i64(t1, t1, 16);
    tcg_gen_or_i64(Rd, t1, t0);

    tcg_temp_free_i64(t0);
    tcg_temp_free_i64(t1);
    tcg_temp_free_i64(mask);

    return true;
}

static bool trans_revh_d(DisasContext *ctx, arg_revh_d *a)
{
    TCGv t0, t1, mask;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();
    mask = tcg_const_tl(0x0000FFFF0000FFFFULL);

    gen_load_gpr(t0, a->rj);

    check_loongarch_64(ctx);
    tcg_gen_shri_tl(t1, t0, 16);
    tcg_gen_and_tl(t1, t1, mask);
    tcg_gen_and_tl(t0, t0, mask);
    tcg_gen_shli_tl(t0, t0, 16);
    tcg_gen_or_tl(t0, t0, t1);
    tcg_gen_shri_tl(t1, t0, 32);
    tcg_gen_shli_tl(t0, t0, 32);
    tcg_gen_or_tl(Rd, t0, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
    tcg_temp_free(mask);

    return true;
}

static bool trans_bitrev_4b(DisasContext *ctx, arg_bitrev_4b *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rj);

    gen_helper_loongarch_bitswap(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_bitrev_8b(DisasContext *ctx, arg_bitrev_8b *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rj);

    check_loongarch_64(ctx);
    gen_helper_loongarch_dbitswap(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_bitrev_w(DisasContext *ctx, arg_bitrev_w *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rj);

    gen_helper_bitrev_w(Rd, cpu_env, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_bitrev_d(DisasContext *ctx, arg_bitrev_d *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_load_gpr(t0, a->rj);

    check_loongarch_64(ctx);
    gen_helper_bitrev_d(Rd, cpu_env, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_bytepick_w(DisasContext *ctx, arg_bytepick_w *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (a->sa2 == 0 || ((a->sa2) * 8) == 32) {
        if (a->sa2 == 0) {
            t0 = get_gpr(a->rk);
        } else {
            t0 = get_gpr(a->rj);
        }
            tcg_gen_ext32s_tl(Rd, t0);
    } else {
        t0 = get_gpr(a->rk);

        TCGv t1 = get_gpr(a->rj);
        TCGv_i64 t2 = tcg_temp_new_i64();

        tcg_gen_concat_tl_i64(t2, t1, t0);
        tcg_gen_shri_i64(t2, t2, 32 - ((a->sa2) * 8));
        tcg_gen_ext32s_i64(Rd, t2);

        tcg_temp_free_i64(t2);
    }

    return true;
}

static bool trans_bytepick_d(DisasContext *ctx, arg_bytepick_d *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();

    check_loongarch_64(ctx);
    if (a->sa3 == 0 || ((a->sa3) * 8) == 64) {
        if (a->sa3 == 0) {
            gen_load_gpr(t0, a->rk);
        } else {
            gen_load_gpr(t0, a->rj);
        }
            tcg_gen_mov_tl(Rd, t0);
    } else {
        TCGv t1 = tcg_temp_new();

        gen_load_gpr(t0, a->rk);
        gen_load_gpr(t1, a->rj);

        tcg_gen_shli_tl(t0, t0, ((a->sa3) * 8));
        tcg_gen_shri_tl(t1, t1, 64 - ((a->sa3) * 8));
        tcg_gen_or_tl(Rd, t1, t0);

        tcg_temp_free(t1);
    }

    tcg_temp_free(t0);

    return true;
}

static bool trans_maskeqz(DisasContext *ctx, arg_maskeqz *a)
{
    TCGv t0, t1, t2;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);
    t2 = tcg_const_tl(0);

    tcg_gen_movcond_tl(TCG_COND_NE, Rd, t0, t2, t1, t2);

    tcg_temp_free(t2);

    return true;
}

static bool trans_masknez(DisasContext *ctx, arg_masknez *a)
{
    TCGv t0, t1, t2;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);
    t2 = tcg_const_tl(0);

    tcg_gen_movcond_tl(TCG_COND_EQ, Rd, t0, t2, t1, t2);

    tcg_temp_free(t2);

    return true;
}

static bool trans_bstrins_d(DisasContext *ctx, arg_bstrins_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    int lsb = a->lsbd;
    int msb = a->msbd;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (lsb > msb) {
        return false;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rd);

    tcg_gen_deposit_tl(t0, t0, t1, lsb, msb - lsb + 1);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_bstrpick_d(DisasContext *ctx, arg_bstrpick_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    int lsb = a->lsbd;
    int msb = a->msbd;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (lsb > msb) {
        return false;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rd);

    tcg_gen_extract_tl(t0, t1, lsb, msb - lsb + 1);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_bstrins_w(DisasContext *ctx, arg_bstrins_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    int lsb = a->lsbw;
    int msb = a->msbw;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if (lsb > msb) {
        return false;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    gen_load_gpr(t0, a->rd);

    tcg_gen_deposit_tl(t0, t0, t1, lsb, msb - lsb + 1);
    tcg_gen_ext32s_tl(t0, t0);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_bstrpick_w(DisasContext *ctx, arg_bstrpick_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    int lsb = a->lsbw;
    int msb = a->msbw;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    if ((a->lsbw > a->msbw) || (lsb + msb > 31)) {
        return false;
    }

    t0 = tcg_temp_new();
    t1 = get_gpr(a->rj);

    if (msb != 31) {
        tcg_gen_extract_tl(t0, t1, lsb, msb + 1);
    } else {
        tcg_gen_ext32s_tl(t0, t1);
    }
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

/* Fixed point load/store instruction translation */
static bool trans_ld_b(DisasContext *ctx, arg_ld_b *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si12);
    tcg_gen_qemu_ld_tl(t0, t0, mem_idx, MO_SB);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_ld_h(DisasContext *ctx, arg_ld_h *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_base_offset_addr(t0, a->rj, a->si12);
    tcg_gen_qemu_ld_tl(t0, t0, mem_idx, MO_TESW |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_ld_w(DisasContext *ctx, arg_ld_w *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_base_offset_addr(t0, a->rj, a->si12);
    tcg_gen_qemu_ld_tl(t0, t0, mem_idx, MO_TESL |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_ld_d(DisasContext *ctx, arg_ld_d *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    gen_base_offset_addr(t0, a->rj, a->si12);
    tcg_gen_qemu_ld_tl(t0, t0, mem_idx, MO_TEQ |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_st_b(DisasContext *ctx, arg_st_b *a)
{
    TCGv t0, t1;
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si12);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_8);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_st_h(DisasContext *ctx, arg_st_h *a)
{
    TCGv t0, t1;
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si12);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_TEUW |
                       ctx->default_tcg_memop_mask);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_st_w(DisasContext *ctx, arg_st_w *a)
{
    TCGv t0, t1;
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si12);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_TEUL |
                       ctx->default_tcg_memop_mask);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_st_d(DisasContext *ctx, arg_st_d *a)
{
    TCGv t0, t1;
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si12);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_TEQ |
                       ctx->default_tcg_memop_mask);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}
static bool trans_ld_bu(DisasContext *ctx, arg_ld_bu *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si12);
    tcg_gen_qemu_ld_tl(t0, t0, mem_idx, MO_UB);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_ld_hu(DisasContext *ctx, arg_ld_hu *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si12);
    tcg_gen_qemu_ld_tl(t0, t0, mem_idx, MO_TEUW |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_ld_wu(DisasContext *ctx, arg_ld_wu *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si12);
    tcg_gen_qemu_ld_tl(t0, t0, mem_idx, MO_TEUL |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_ldx_b(DisasContext *ctx, arg_ldx_b *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    tcg_gen_qemu_ld_tl(t1, t0, mem_idx, MO_SB);
    tcg_gen_mov_tl(Rd, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_ldx_h(DisasContext *ctx, arg_ldx_h *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    tcg_gen_qemu_ld_tl(t1, t0, mem_idx, MO_TESW |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_ldx_w(DisasContext *ctx, arg_ldx_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    tcg_gen_qemu_ld_tl(t1, t0, mem_idx, MO_TESL |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_ldx_d(DisasContext *ctx, arg_ldx_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    tcg_gen_qemu_ld_tl(t1, t0, mem_idx, MO_TEQ |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_stx_b(DisasContext *ctx, arg_stx_b *a)
{
    TCGv t0, t1;
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_8);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_stx_h(DisasContext *ctx, arg_stx_h *a)
{
    TCGv t0, t1;
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_TEUW |
                       ctx->default_tcg_memop_mask);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_stx_w(DisasContext *ctx, arg_stx_w *a)
{
    TCGv t0, t1;
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_TEUL |
                       ctx->default_tcg_memop_mask);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_stx_d(DisasContext *ctx, arg_stx_d *a)
{
    TCGv t0, t1;
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_TEQ |
                       ctx->default_tcg_memop_mask);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_ldx_bu(DisasContext *ctx, arg_ldx_bu *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    tcg_gen_qemu_ld_tl(t1, t0, mem_idx, MO_UB);
    tcg_gen_mov_tl(Rd, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_ldx_hu(DisasContext *ctx, arg_ldx_hu *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    tcg_gen_qemu_ld_tl(t1, t0, mem_idx, MO_TEUW |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_ldx_wu(DisasContext *ctx, arg_ldx_wu *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv Rj = cpu_gpr[a->rj];
    TCGv Rk = cpu_gpr[a->rk];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_op_addr_add(t0, Rj, Rk);
    tcg_gen_qemu_ld_tl(t1, t0, mem_idx, MO_TEUL |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_preld(DisasContext *ctx, arg_preld *a)
{
    /* Treat as NOP. */
    return true;
}

static bool trans_dbar(DisasContext *ctx, arg_dbar * a)
{
    gen_loongarch_sync(a->whint);
    return true;
}

static bool trans_ibar(DisasContext *ctx, arg_ibar *a)
{
    /*
     * IBAR is a no-op in QEMU,
     * however we need to end the translation block
     */
    ctx->base.is_jmp = DISAS_STOP;
    return true;
}

static bool trans_ldptr_w(DisasContext *ctx, arg_ldptr_w *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si14 << 2);
    tcg_gen_qemu_ld_tl(t0, t0, mem_idx, MO_TESL |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_stptr_w(DisasContext *ctx, arg_stptr_w *a)
{
    TCGv t0, t1;
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si14 << 2);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_TEUL |
                       ctx->default_tcg_memop_mask);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_ldptr_d(DisasContext *ctx, arg_ldptr_d *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];
    int mem_idx = ctx->mem_idx;

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si14 << 2);
    tcg_gen_qemu_ld_tl(t0, t0, mem_idx, MO_TEQ |
                       ctx->default_tcg_memop_mask);
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);

    return true;
}

static bool trans_stptr_d(DisasContext *ctx, arg_stptr_d *a)
{
    TCGv t0, t1;
    int mem_idx = ctx->mem_idx;

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si14 << 2);
    gen_load_gpr(t1, a->rd);
    tcg_gen_qemu_st_tl(t1, t0, mem_idx, MO_TEQ |
                       ctx->default_tcg_memop_mask);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

#define ASRTGT                                \
    do {                                      \
        TCGv t1 = get_gpr(a->rj);             \
        TCGv t2 = get_gpr(a->rk);             \
        gen_helper_asrtgt_d(cpu_env, t1, t2); \
    } while (0)

#define ASRTLE                                \
    do {                                      \
        TCGv t1 = get_gpr(a->rj);             \
        TCGv t2 = get_gpr(a->rk);             \
        gen_helper_asrtle_d(cpu_env, t1, t2); \
    } while (0)

#define DECL_ARG(name)   \
    arg_ ## name arg = { \
        .rd = a->rd,     \
        .rj = a->rj,     \
        .rk = a->rk,     \
    };

static bool trans_ldgt_b(DisasContext *ctx, arg_ldgt_b *a)
{
    ASRTGT;
    DECL_ARG(ldx_b)
    trans_ldx_b(ctx, &arg);
    return true;
}

static bool trans_ldgt_h(DisasContext *ctx, arg_ldgt_h *a)
{
    ASRTGT;
    DECL_ARG(ldx_h)
    trans_ldx_h(ctx, &arg);
    return true;
}

static bool trans_ldgt_w(DisasContext *ctx, arg_ldgt_w *a)
{
    ASRTGT;
    DECL_ARG(ldx_w)
    trans_ldx_w(ctx, &arg);
    return true;
}

static bool trans_ldgt_d(DisasContext *ctx, arg_ldgt_d *a)
{
    ASRTGT;
    DECL_ARG(ldx_d)
    trans_ldx_d(ctx, &arg);
    return true;
}

static bool trans_ldle_b(DisasContext *ctx, arg_ldle_b *a)
{
    ASRTLE;
    DECL_ARG(ldx_b)
    trans_ldx_b(ctx, &arg);
    return true;
}

static bool trans_ldle_h(DisasContext *ctx, arg_ldle_h *a)
{
    ASRTLE;
    DECL_ARG(ldx_h)
    trans_ldx_h(ctx, &arg);
    return true;
}

static bool trans_ldle_w(DisasContext *ctx, arg_ldle_w *a)
{
    ASRTLE;
    DECL_ARG(ldx_w)
    trans_ldx_w(ctx, &arg);
    return true;
}

static bool trans_ldle_d(DisasContext *ctx, arg_ldle_d *a)
{
    ASRTLE;
    DECL_ARG(ldx_d)
    trans_ldx_d(ctx, &arg);
    return true;
}

static bool trans_stgt_b(DisasContext *ctx, arg_stgt_b *a)
{
    ASRTGT;
    DECL_ARG(stx_b)
    trans_stx_b(ctx, &arg);
    return true;
}

static bool trans_stgt_h(DisasContext *ctx, arg_stgt_h *a)
{
    ASRTGT;
    DECL_ARG(stx_h)
    trans_stx_h(ctx, &arg);
    return true;
}

static bool trans_stgt_w(DisasContext *ctx, arg_stgt_w *a)
{
    ASRTGT;
    DECL_ARG(stx_w)
    trans_stx_w(ctx, &arg);
    return true;
}

static bool trans_stgt_d(DisasContext *ctx, arg_stgt_d *a)
{
    ASRTGT;
    DECL_ARG(stx_d)
    trans_stx_d(ctx, &arg);
    return true;
}

static bool trans_stle_b(DisasContext *ctx, arg_stle_b *a)
{
    ASRTLE;
    DECL_ARG(stx_b)
    trans_stx_b(ctx, &arg);
    return true;
}

static bool trans_stle_h(DisasContext *ctx, arg_stle_h *a)
{
    ASRTLE;
    DECL_ARG(stx_h)
    trans_stx_h(ctx, &arg);
    return true;
}

static bool trans_stle_w(DisasContext *ctx, arg_stle_w *a)
{
    ASRTLE;
    DECL_ARG(stx_w)
    trans_stx_w(ctx, &arg);
    return true;
}

static bool trans_stle_d(DisasContext *ctx, arg_stle_d *a)
{
    ASRTLE;
    DECL_ARG(stx_d)
    trans_stx_d(ctx, &arg);
    return true;
}

#undef DECL_ARG

/* Fixed point atomic instruction translation */
static bool trans_ll_w(DisasContext *ctx, arg_ll_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si14 << 2);
    tcg_gen_mov_tl(t1, t0);
    tcg_gen_qemu_ld32s(t0, t0, ctx->mem_idx);
    tcg_gen_st_tl(t1, cpu_env, offsetof(CPULoongArchState, lladdr));
    tcg_gen_st_tl(t0, cpu_env, offsetof(CPULoongArchState, llval));
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_sc_w(DisasContext *ctx, arg_sc_w *a)
{
    gen_loongarch_st_cond(ctx, a->rd, a->rj, a->si14 << 2, MO_TESL, false);
    return true;
}

static bool trans_ll_d(DisasContext *ctx, arg_ll_d *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = tcg_temp_new();
    t1 = tcg_temp_new();

    gen_base_offset_addr(t0, a->rj, a->si14 << 2);
    tcg_gen_mov_tl(t1, t0);
    tcg_gen_qemu_ld64(t0, t0, ctx->mem_idx);
    tcg_gen_st_tl(t1, cpu_env, offsetof(CPULoongArchState, lladdr));
    tcg_gen_st_tl(t0, cpu_env, offsetof(CPULoongArchState, llval));
    tcg_gen_mov_tl(Rd, t0);

    tcg_temp_free(t0);
    tcg_temp_free(t1);

    return true;
}

static bool trans_sc_d(DisasContext *ctx, arg_sc_d *a)
{
    gen_loongarch_st_cond(ctx, a->rd, a->rj, a->si14 << 2, MO_TEQ, false);
    return true;
}

#define TRANS_AM_W(name, op)                                      \
static bool trans_ ## name(DisasContext *ctx, arg_ ## name * a)   \
{                                                                 \
    TCGv addr, val, ret;                                          \
    TCGv Rd = cpu_gpr[a->rd];                                     \
    int mem_idx = ctx->mem_idx;                                   \
                                                                  \
    if (a->rd == 0) {                                             \
        return true;                                              \
    }                                                             \
    if ((a->rd != 0) && ((a->rj == a->rd) || (a->rk == a->rd))) { \
        printf("%s: warning, register equal\n", __func__);        \
        return false;                                             \
    }                                                             \
                                                                  \
    addr = get_gpr(a->rj);                                        \
    val = get_gpr(a->rk);                                         \
    ret = tcg_temp_new();                                         \
                                                                  \
    tcg_gen_atomic_##op##_tl(ret, addr, val, mem_idx, MO_TESL |   \
                            ctx->default_tcg_memop_mask);         \
    tcg_gen_mov_tl(Rd, ret);                                      \
                                                                  \
    tcg_temp_free(ret);                                           \
                                                                  \
    return true;                                                  \
}
#define TRANS_AM_D(name, op)                                      \
static bool trans_ ## name(DisasContext *ctx, arg_ ## name * a)   \
{                                                                 \
    TCGv addr, val, ret;                                          \
    TCGv Rd = cpu_gpr[a->rd];                                     \
    int mem_idx = ctx->mem_idx;                                   \
                                                                  \
    if (a->rd == 0) {                                             \
        return true;                                              \
    }                                                             \
    if ((a->rd != 0) && ((a->rj == a->rd) || (a->rk == a->rd))) { \
        printf("%s: warning, register equal\n", __func__);        \
        return false;                                             \
    }                                                             \
    addr = get_gpr(a->rj);                                        \
    val = get_gpr(a->rk);                                         \
    ret = tcg_temp_new();                                         \
                                                                  \
    tcg_gen_atomic_##op##_tl(ret, addr, val, mem_idx, MO_TEQ |    \
                            ctx->default_tcg_memop_mask);         \
    tcg_gen_mov_tl(Rd, ret);                                      \
                                                                  \
    tcg_temp_free(ret);                                           \
                                                                  \
    return true;                                                  \
}
#define TRANS_AM(name, op)   \
    TRANS_AM_W(name##_w, op) \
    TRANS_AM_D(name##_d, op)
TRANS_AM(amswap, xchg)      /* trans_amswap_w, trans_amswap_d */
TRANS_AM(amadd, fetch_add)  /* trans_amadd_w, trans_amadd_d   */
TRANS_AM(amand, fetch_and)  /* trans_amand_w, trans_amand_d   */
TRANS_AM(amor, fetch_or)    /* trans_amor_w, trans_amor_d     */
TRANS_AM(amxor, fetch_xor)  /* trans_amxor_w, trans_amxor_d   */
TRANS_AM(ammax, fetch_smax) /* trans_ammax_w, trans_ammax_d   */
TRANS_AM(ammin, fetch_smin) /* trans_ammin_w, trans_ammin_d   */
TRANS_AM_W(ammax_wu, fetch_umax)    /* trans_ammax_wu */
TRANS_AM_D(ammax_du, fetch_umax)    /* trans_ammax_du */
TRANS_AM_W(ammin_wu, fetch_umin)    /* trans_ammin_wu */
TRANS_AM_D(ammin_du, fetch_umin)    /* trans_ammin_du */
#undef TRANS_AM
#undef TRANS_AM_W
#undef TRANS_AM_D

#define TRANS_AM_DB_W(name, op)                                   \
static bool trans_ ## name(DisasContext *ctx, arg_ ## name * a)   \
{                                                                 \
    TCGv addr, val, ret;                                          \
    TCGv Rd = cpu_gpr[a->rd];                                     \
    int mem_idx = ctx->mem_idx;                                   \
                                                                  \
    if (a->rd == 0) {                                             \
        return true;                                              \
    }                                                             \
    if ((a->rd != 0) && ((a->rj == a->rd) || (a->rk == a->rd))) { \
        printf("%s: warning, register equal\n", __func__);        \
        return false;                                             \
    }                                                             \
                                                                  \
    addr = get_gpr(a->rj);                                        \
    val = get_gpr(a->rk);                                         \
    ret = tcg_temp_new();                                         \
                                                                  \
    gen_loongarch_sync(0x10);                                     \
    tcg_gen_atomic_##op##_tl(ret, addr, val, mem_idx, MO_TESL |   \
                            ctx->default_tcg_memop_mask);         \
    tcg_gen_mov_tl(Rd, ret);                                      \
                                                                  \
    tcg_temp_free(ret);                                           \
                                                                  \
    return true;                                                  \
}
#define TRANS_AM_DB_D(name, op)                                   \
static bool trans_ ## name(DisasContext *ctx, arg_ ## name * a)   \
{                                                                 \
    TCGv addr, val, ret;                                          \
    TCGv Rd = cpu_gpr[a->rd];                                     \
    int mem_idx = ctx->mem_idx;                                   \
                                                                  \
    if (a->rd == 0) {                                             \
        return true;                                              \
    }                                                             \
    if ((a->rd != 0) && ((a->rj == a->rd) || (a->rk == a->rd))) { \
        printf("%s: warning, register equal\n", __func__);        \
        return false;                                             \
    }                                                             \
                                                                  \
    addr = get_gpr(a->rj);                                        \
    val = get_gpr(a->rk);                                         \
    ret = tcg_temp_new();                                         \
                                                                  \
    gen_loongarch_sync(0x10);                                     \
    tcg_gen_atomic_##op##_tl(ret, addr, val, mem_idx, MO_TEQ |    \
                            ctx->default_tcg_memop_mask);         \
    tcg_gen_mov_tl(Rd, ret);                                      \
                                                                  \
    tcg_temp_free(ret);                                           \
                                                                  \
    return true;                                                  \
}
#define TRANS_AM_DB(name, op)      \
    TRANS_AM_DB_W(name##_db_w, op) \
    TRANS_AM_DB_D(name##_db_d, op)
TRANS_AM_DB(amswap, xchg)      /* trans_amswap_db_w, trans_amswap_db_d */
TRANS_AM_DB(amadd, fetch_add)  /* trans_amadd_db_w, trans_amadd_db_d   */
TRANS_AM_DB(amand, fetch_and)  /* trans_amand_db_w, trans_amand_db_d   */
TRANS_AM_DB(amor, fetch_or)    /* trans_amor_db_w, trans_amor_db_d     */
TRANS_AM_DB(amxor, fetch_xor)  /* trans_amxor_db_w, trans_amxor_db_d   */
TRANS_AM_DB(ammax, fetch_smax) /* trans_ammax_db_w, trans_ammax_db_d   */
TRANS_AM_DB(ammin, fetch_smin) /* trans_ammin_db_w, trans_ammin_db_d   */
TRANS_AM_DB_W(ammax_db_wu, fetch_umax)    /* trans_ammax_db_wu */
TRANS_AM_DB_D(ammax_db_du, fetch_umax)    /* trans_ammax_db_du */
TRANS_AM_DB_W(ammin_db_wu, fetch_umin)    /* trans_ammin_db_wu */
TRANS_AM_DB_D(ammin_db_du, fetch_umin)    /* trans_ammin_db_du */
#undef TRANS_AM_DB
#undef TRANS_AM_DB_W
#undef TRANS_AM_DB_D

/* Fixed point extra instruction translation */
static bool trans_crc_w_b_w(DisasContext *ctx, arg_crc_w_b_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv_i32 tsz = tcg_const_i32(1 << 1);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);

    gen_helper_crc32(Rd, t0, t1, tsz);

    tcg_temp_free_i32(tsz);

    return true;
}

static bool trans_crc_w_h_w(DisasContext *ctx, arg_crc_w_h_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv_i32 tsz = tcg_const_i32(1 << 2);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);

    gen_helper_crc32(Rd, t0, t1, tsz);

    tcg_temp_free_i32(tsz);

    return true;
}

static bool trans_crc_w_w_w(DisasContext *ctx, arg_crc_w_w_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv_i32 tsz = tcg_const_i32(1 << 4);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);

    gen_helper_crc32(Rd, t0, t1, tsz);

    tcg_temp_free_i32(tsz);

    return true;
}

static bool trans_crc_w_d_w(DisasContext *ctx, arg_crc_w_d_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv_i32 tsz = tcg_const_i32(1 << 8);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);

    gen_helper_crc32(Rd, t0, t1, tsz);

    tcg_temp_free_i32(tsz);

    return true;
}

static bool trans_crcc_w_b_w(DisasContext *ctx, arg_crcc_w_b_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv_i32 tsz = tcg_const_i32(1 << 1);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);

    gen_helper_crc32c(Rd, t0, t1, tsz);

    tcg_temp_free_i32(tsz);

    return true;
}

static bool trans_crcc_w_h_w(DisasContext *ctx, arg_crcc_w_h_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv_i32 tsz = tcg_const_i32(1 << 2);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);

    gen_helper_crc32c(Rd, t0, t1, tsz);

    tcg_temp_free_i32(tsz);

    return true;
}

static bool trans_crcc_w_w_w(DisasContext *ctx, arg_crcc_w_w_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv_i32 tsz = tcg_const_i32(1 << 4);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);

    gen_helper_crc32c(Rd, t0, t1, tsz);

    tcg_temp_free_i32(tsz);

    return true;
}

static bool trans_crcc_w_d_w(DisasContext *ctx, arg_crcc_w_d_w *a)
{
    TCGv t0, t1;
    TCGv Rd = cpu_gpr[a->rd];
    TCGv_i32 tsz = tcg_const_i32(1 << 8);

    if (a->rd == 0) {
        /* Nop */
        return true;
    }

    t0 = get_gpr(a->rk);
    t1 = get_gpr(a->rj);

    gen_helper_crc32c(Rd, t0, t1, tsz);

    tcg_temp_free_i32(tsz);

    return true;
}

static bool trans_break(DisasContext *ctx, arg_break *a)
{
    generate_exception_end(ctx, EXCP_BREAK);
    return true;
}

static bool trans_syscall(DisasContext *ctx, arg_syscall *a)
{
    generate_exception_end(ctx, EXCP_SYSCALL);
    return true;
}

static bool trans_asrtle_d(DisasContext *ctx, arg_asrtle_d * a)
{
    TCGv t0, t1;

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);

    gen_helper_asrtle_d(cpu_env, t0, t1);

    return true;
}

static bool trans_asrtgt_d(DisasContext *ctx, arg_asrtgt_d * a)
{
    TCGv t0, t1;

    t0 = get_gpr(a->rj);
    t1 = get_gpr(a->rk);

    gen_helper_asrtgt_d(cpu_env, t0, t1);

    return true;
}

static bool trans_rdtimel_w(DisasContext *ctx, arg_rdtimel_w *a)
{
    /* Nop */
    return true;
}

static bool trans_rdtimeh_w(DisasContext *ctx, arg_rdtimeh_w *a)
{
    /* Nop */
    return true;
}

static bool trans_rdtime_d(DisasContext *ctx, arg_rdtime_d *a)
{
    /* Nop */
    return true;
}

static bool trans_cpucfg(DisasContext *ctx, arg_cpucfg *a)
{
    TCGv t0;
    TCGv Rd = cpu_gpr[a->rd];

    t0 = get_gpr(a->rj);

    gen_helper_cpucfg(Rd, cpu_env, t0);

    return true;
}

/* Floating point arithmetic operation instruction translation */
static bool trans_fadd_s(DisasContext *ctx, arg_fadd_s * a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_helper_fp_add_s(fp0, cpu_env, fp0, fp1);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_fadd_d(DisasContext *ctx, arg_fadd_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_helper_fp_add_d(fp0, cpu_env, fp0, fp1);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_fsub_s(DisasContext *ctx, arg_fsub_s *a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_helper_fp_sub_s(fp0, cpu_env, fp0, fp1);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_fsub_d(DisasContext *ctx, arg_fsub_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_helper_fp_sub_d(fp0, cpu_env, fp0, fp1);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_fmul_s(DisasContext *ctx, arg_fmul_s *a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_helper_fp_mul_s(fp0, cpu_env, fp0, fp1);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_fmul_d(DisasContext *ctx, arg_fmul_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_helper_fp_mul_d(fp0, cpu_env, fp0, fp1);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_fdiv_s(DisasContext *ctx, arg_fdiv_s *a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_helper_fp_div_s(fp0, cpu_env, fp0, fp1);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_fdiv_d(DisasContext *ctx, arg_fdiv_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_helper_fp_div_d(fp0, cpu_env, fp0, fp1);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_fmadd_s(DisasContext *ctx, arg_fmadd_s *a)
{
    TCGv_i32 fp0, fp1, fp2, fp3;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();
    fp2 = tcg_temp_new_i32();
    fp3 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_load_fpr32(fp2, a->fa);
    gen_helper_fp_madd_s(fp3, cpu_env, fp0, fp1, fp2);
    gen_store_fpr32(fp3, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);
    tcg_temp_free_i32(fp2);
    tcg_temp_free_i32(fp3);

    return true;
}

static bool trans_fmadd_d(DisasContext *ctx, arg_fmadd_d *a)
{
    TCGv_i64 fp0, fp1, fp2, fp3;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();
    fp2 = tcg_temp_new_i64();
    fp3 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_load_fpr64(fp2, a->fa);
    check_fpu_enabled(ctx);
    gen_helper_fp_madd_d(fp3, cpu_env, fp0, fp1, fp2);
    gen_store_fpr64(fp3, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);
    tcg_temp_free_i64(fp2);
    tcg_temp_free_i64(fp3);

    return true;
}

static bool trans_fmsub_s(DisasContext *ctx, arg_fmsub_s *a)
{
    TCGv_i32 fp0, fp1, fp2, fp3;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();
    fp2 = tcg_temp_new_i32();
    fp3 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_load_fpr32(fp2, a->fa);
    gen_helper_fp_msub_s(fp3, cpu_env, fp0, fp1, fp2);
    gen_store_fpr32(fp3, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);
    tcg_temp_free_i32(fp2);
    tcg_temp_free_i32(fp3);

    return true;
}

static bool trans_fmsub_d(DisasContext *ctx, arg_fmsub_d *a)
{
    TCGv_i64 fp0, fp1, fp2, fp3;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();
    fp2 = tcg_temp_new_i64();
    fp3 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_load_fpr64(fp2, a->fa);
    gen_helper_fp_msub_d(fp3, cpu_env, fp0, fp1, fp2);
    gen_store_fpr64(fp3, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);
    tcg_temp_free_i64(fp2);
    tcg_temp_free_i64(fp3);

    return true;
}

static bool trans_fnmadd_s(DisasContext *ctx, arg_fnmadd_s *a)
{
    TCGv_i32 fp0, fp1, fp2, fp3;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();
    fp2 = tcg_temp_new_i32();
    fp3 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_load_fpr32(fp2, a->fa);
    gen_helper_fp_nmadd_s(fp3, cpu_env, fp0, fp1, fp2);
    gen_store_fpr32(fp3, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);
    tcg_temp_free_i32(fp2);
    tcg_temp_free_i32(fp3);

    return true;
}

static bool trans_fnmadd_d(DisasContext *ctx, arg_fnmadd_d *a)
{
    TCGv_i64 fp0, fp1, fp2, fp3;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();
    fp2 = tcg_temp_new_i64();
    fp3 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_load_fpr64(fp2, a->fa);
    gen_helper_fp_nmadd_d(fp3, cpu_env, fp0, fp1, fp2);
    gen_store_fpr64(fp3, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);
    tcg_temp_free_i64(fp2);
    tcg_temp_free_i64(fp3);

    return true;
}

static bool trans_fnmsub_s(DisasContext *ctx, arg_fnmsub_s *a)
{
    TCGv_i32 fp0, fp1, fp2, fp3;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();
    fp2 = tcg_temp_new_i32();
    fp3 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_load_fpr32(fp2, a->fa);
    gen_helper_fp_nmsub_s(fp3, cpu_env, fp0, fp1, fp2);
    gen_store_fpr32(fp3, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);
    tcg_temp_free_i32(fp2);
    tcg_temp_free_i32(fp3);

    return true;
}

static bool trans_fnmsub_d(DisasContext *ctx, arg_fnmsub_d *a)
{
    TCGv_i64 fp0, fp1, fp2, fp3;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();
    fp2 = tcg_temp_new_i64();
    fp3 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_load_fpr64(fp2, a->fa);
    gen_helper_fp_nmsub_d(fp3, cpu_env, fp0, fp1, fp2);
    gen_store_fpr64(fp3, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);
    tcg_temp_free_i64(fp2);
    tcg_temp_free_i64(fp3);

    return true;
}

static bool trans_fmax_s(DisasContext *ctx, arg_fmax_s *a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_helper_fp_max_s(fp1, cpu_env, fp0, fp1);
    gen_store_fpr32(fp1, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_fmax_d(DisasContext *ctx, arg_fmax_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_helper_fp_max_d(fp1, cpu_env, fp0, fp1);
    gen_store_fpr64(fp1, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_fmin_s(DisasContext *ctx, arg_fmin_s *a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_helper_fp_min_s(fp1, cpu_env, fp0, fp1);
    gen_store_fpr32(fp1, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_fmin_d(DisasContext *ctx, arg_fmin_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_helper_fp_min_d(fp1, cpu_env, fp0, fp1);
    gen_store_fpr64(fp1, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_fmaxa_s(DisasContext *ctx, arg_fmaxa_s *a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_helper_fp_maxa_s(fp1, cpu_env, fp0, fp1);
    gen_store_fpr32(fp1, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_fmaxa_d(DisasContext *ctx, arg_fmaxa_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_helper_fp_maxa_d(fp1, cpu_env, fp0, fp1);
    gen_store_fpr64(fp1, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_fmina_s(DisasContext *ctx, arg_fmina_s *a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_helper_fp_mina_s(fp1, cpu_env, fp0, fp1);
    gen_store_fpr32(fp1, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_fmina_d(DisasContext *ctx, arg_fmina_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_helper_fp_mina_d(fp1, cpu_env, fp0, fp1);
    gen_store_fpr64(fp1, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_fabs_s(DisasContext *ctx, arg_fabs_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_abs_s(fp0, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_fabs_d(DisasContext *ctx, arg_fabs_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_abs_d(fp0, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_fneg_s(DisasContext *ctx, arg_fneg_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_neg_s(fp0, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_fneg_d(DisasContext *ctx, arg_fneg_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_neg_d(fp0, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_fsqrt_s(DisasContext *ctx, arg_fsqrt_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_sqrt_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_fsqrt_d(DisasContext *ctx, arg_fsqrt_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_sqrt_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_frecip_s(DisasContext *ctx, arg_frecip_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_recip_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_frecip_d(DisasContext *ctx, arg_frecip_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_recip_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_frsqrt_s(DisasContext *ctx, arg_frsqrt_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_rsqrt_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_frsqrt_d(DisasContext *ctx, arg_frsqrt_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_rsqrt_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_fscaleb_s(DisasContext *ctx, arg_fscaleb_s *a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    gen_helper_fp_exp2_s(fp0, cpu_env, fp0, fp1);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_fscaleb_d(DisasContext *ctx, arg_fscaleb_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    gen_helper_fp_exp2_d(fp0, cpu_env, fp0, fp1);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_flogb_s(DisasContext *ctx, arg_flogb_s *a)
{
    TCGv_i32 fp0, fp1;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_logb_s(fp1, cpu_env, fp0);
    gen_store_fpr32(fp1, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);

    return true;
}

static bool trans_flogb_d(DisasContext *ctx, arg_flogb_d *a)
{
    TCGv_i64 fp0, fp1;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_logb_d(fp1, cpu_env, fp0);
    gen_store_fpr64(fp1, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);

    return true;
}

static bool trans_fcopysign_s(DisasContext *ctx, arg_fcopysign_s *a)
{
    TCGv_i32 fp0, fp1, fp2;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();
    fp2 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);
    tcg_gen_deposit_i32(fp2, fp1, fp0, 0, 31);
    gen_store_fpr32(fp2, a->fd);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);
    tcg_temp_free_i32(fp2);

    return true;
}

static bool trans_fcopysign_d(DisasContext *ctx, arg_fcopysign_d *a)
{
    TCGv_i64 fp0, fp1, fp2;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();
    fp2 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);
    tcg_gen_deposit_i64(fp2, fp1, fp0, 0, 63);
    gen_store_fpr64(fp2, a->fd);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);
    tcg_temp_free_i64(fp2);

    return true;
}

static bool trans_fclass_s(DisasContext *ctx, arg_fclass_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_class_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_fclass_d(DisasContext *ctx, arg_fclass_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_class_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

/* Floating point compare instruction translation */
static bool trans_fcmp_cond_s(DisasContext *ctx, arg_fcmp_cond_s *a)
{
    TCGv_i32 fp0, fp1, fcc;

    fp0 = tcg_temp_new_i32();
    fp1 = tcg_temp_new_i32();
    fcc = tcg_const_i32(a->cd);

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_load_fpr32(fp1, a->fk);

    switch (a->fcond) {
    case  0:
        gen_helper_fp_cmp_caf_s(fp0, cpu_env, fp0, fp1);
        break;
    case  1:
        gen_helper_fp_cmp_saf_s(fp0, cpu_env, fp0, fp1);
        break;
    case  2:
        gen_helper_fp_cmp_clt_s(fp0, cpu_env, fp0, fp1);
        break;
    case  3:
        gen_helper_fp_cmp_slt_s(fp0, cpu_env, fp0, fp1);
        break;
    case  4:
        gen_helper_fp_cmp_ceq_s(fp0, cpu_env, fp0, fp1);
        break;
    case  5:
        gen_helper_fp_cmp_seq_s(fp0, cpu_env, fp0, fp1);
        break;
    case  6:
        gen_helper_fp_cmp_cle_s(fp0, cpu_env, fp0, fp1);
        break;
    case  7:
        gen_helper_fp_cmp_sle_s(fp0, cpu_env, fp0, fp1);
        break;
    case  8:
        gen_helper_fp_cmp_cun_s(fp0, cpu_env, fp0, fp1);
        break;
    case  9:
        gen_helper_fp_cmp_sun_s(fp0, cpu_env, fp0, fp1);
        break;
    case 10:
        gen_helper_fp_cmp_cult_s(fp0, cpu_env, fp0, fp1);
        break;
    case 11:
        gen_helper_fp_cmp_sult_s(fp0, cpu_env, fp0, fp1);
        break;
    case 12:
        gen_helper_fp_cmp_cueq_s(fp0, cpu_env, fp0, fp1);
        break;
    case 13:
        gen_helper_fp_cmp_sueq_s(fp0, cpu_env, fp0, fp1);
        break;
    case 14:
        gen_helper_fp_cmp_cule_s(fp0, cpu_env, fp0, fp1);
        break;
    case 15:
        gen_helper_fp_cmp_sule_s(fp0, cpu_env, fp0, fp1);
        break;
    case 16:
        gen_helper_fp_cmp_cne_s(fp0, cpu_env, fp0, fp1);
        break;
    case 17:
        gen_helper_fp_cmp_sne_s(fp0, cpu_env, fp0, fp1);
        break;
    case 20:
        gen_helper_fp_cmp_cor_s(fp0, cpu_env, fp0, fp1);
        break;
    case 21:
        gen_helper_fp_cmp_sor_s(fp0, cpu_env, fp0, fp1);
        break;
    case 24:
        gen_helper_fp_cmp_cune_s(fp0, cpu_env, fp0, fp1);
        break;
    case 25:
        gen_helper_fp_cmp_sune_s(fp0, cpu_env, fp0, fp1);
        break;
    default:
        abort();
    }
    gen_helper_movreg2cf_i32(cpu_env, fcc, fp0);

    tcg_temp_free_i32(fp0);
    tcg_temp_free_i32(fp1);
    tcg_temp_free_i32(fcc);

    return true;
}

static bool trans_fcmp_cond_d(DisasContext *ctx, arg_fcmp_cond_d *a)
{
    TCGv_i64 fp0, fp1;
    TCGv_i32 fcc;

    fp0 = tcg_temp_new_i64();
    fp1 = tcg_temp_new_i64();
    fcc = tcg_const_i32(a->cd);

    gen_load_fpr64(fp0, a->fj);
    gen_load_fpr64(fp1, a->fk);

    check_fpu_enabled(ctx);
    switch (a->fcond) {
    case  0:
        gen_helper_fp_cmp_caf_d(fp0, cpu_env, fp0, fp1);
        break;
    case  1:
        gen_helper_fp_cmp_saf_d(fp0, cpu_env, fp0, fp1);
        break;
    case  2:
        gen_helper_fp_cmp_clt_d(fp0, cpu_env, fp0, fp1);
        break;
    case  3:
        gen_helper_fp_cmp_slt_d(fp0, cpu_env, fp0, fp1);
        break;
    case  4:
        gen_helper_fp_cmp_ceq_d(fp0, cpu_env, fp0, fp1);
        break;
    case  5:
        gen_helper_fp_cmp_seq_d(fp0, cpu_env, fp0, fp1);
        break;
    case  6:
        gen_helper_fp_cmp_cle_d(fp0, cpu_env, fp0, fp1);
        break;
    case  7:
        gen_helper_fp_cmp_sle_d(fp0, cpu_env, fp0, fp1);
        break;
    case  8:
        gen_helper_fp_cmp_cun_d(fp0, cpu_env, fp0, fp1);
        break;
    case  9:
        gen_helper_fp_cmp_sun_d(fp0, cpu_env, fp0, fp1);
        break;
    case 10:
        gen_helper_fp_cmp_cult_d(fp0, cpu_env, fp0, fp1);
        break;
    case 11:
        gen_helper_fp_cmp_sult_d(fp0, cpu_env, fp0, fp1);
        break;
    case 12:
        gen_helper_fp_cmp_cueq_d(fp0, cpu_env, fp0, fp1);
        break;
    case 13:
        gen_helper_fp_cmp_sueq_d(fp0, cpu_env, fp0, fp1);
        break;
    case 14:
        gen_helper_fp_cmp_cule_d(fp0, cpu_env, fp0, fp1);
        break;
    case 15:
        gen_helper_fp_cmp_sule_d(fp0, cpu_env, fp0, fp1);
        break;
    case 16:
        gen_helper_fp_cmp_cne_d(fp0, cpu_env, fp0, fp1);
        break;
    case 17:
        gen_helper_fp_cmp_sne_d(fp0, cpu_env, fp0, fp1);
        break;
    case 20:
        gen_helper_fp_cmp_cor_d(fp0, cpu_env, fp0, fp1);
        break;
    case 21:
        gen_helper_fp_cmp_sor_d(fp0, cpu_env, fp0, fp1);
        break;
    case 24:
        gen_helper_fp_cmp_cune_d(fp0, cpu_env, fp0, fp1);
        break;
    case 25:
        gen_helper_fp_cmp_sune_d(fp0, cpu_env, fp0, fp1);
        break;
    default:
        abort();
    }
    gen_helper_movreg2cf_i64(cpu_env, fcc, fp0);

    tcg_temp_free_i64(fp0);
    tcg_temp_free_i64(fp1);
    tcg_temp_free_i32(fcc);

    return true;
}

/* Floating point conversion instruction */
static bool trans_fcvt_s_d(DisasContext *ctx, arg_fcvt_s_d *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp64, a->fj);
    gen_helper_fp_cvt_s_d(fp32, cpu_env, fp64);
    gen_store_fpr32(fp32, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_fcvt_d_s(DisasContext *ctx, arg_fcvt_d_s *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp32, a->fj);
    gen_helper_fp_cvt_d_s(fp64, cpu_env, fp32);
    gen_store_fpr64(fp64, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftintrm_w_s(DisasContext *ctx, arg_ftintrm_l_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_tintrm_w_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_ftintrm_w_d(DisasContext *ctx, arg_ftintrm_l_d *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp64, a->fj);
    gen_helper_fp_tintrm_w_d(fp32, cpu_env, fp64);
    gen_store_fpr32(fp32, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftintrm_l_s(DisasContext *ctx, arg_ftintrm_l_s *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp32, a->fj);
    gen_helper_fp_tintrm_l_s(fp64, cpu_env, fp32);
    gen_store_fpr64(fp64, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftintrm_l_d(DisasContext *ctx, arg_ftintrm_l_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_tintrm_l_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_ftintrp_w_s(DisasContext *ctx, arg_ftintrp_w_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_tintrp_w_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_ftintrp_w_d(DisasContext *ctx, arg_ftintrp_w_d *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp64, a->fj);
    gen_helper_fp_tintrp_w_d(fp32, cpu_env, fp64);
    gen_store_fpr32(fp32, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftintrp_l_s(DisasContext *ctx, arg_ftintrp_l_s *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp32, a->fj);
    gen_helper_fp_tintrp_l_s(fp64, cpu_env, fp32);
    gen_store_fpr64(fp64, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftintrp_l_d(DisasContext *ctx, arg_ftintrp_l_d *a)
{
    TCGv_i64 fp0;

    fp0  = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_tintrp_l_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_ftintrz_w_s(DisasContext *ctx, arg_ftintrz_w_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_tintrz_w_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_ftintrz_w_d(DisasContext *ctx, arg_ftintrz_w_d *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp64, a->fj);
    gen_helper_fp_tintrz_w_d(fp32, cpu_env, fp64);
    gen_store_fpr32(fp32, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftintrz_l_s(DisasContext *ctx, arg_ftintrz_l_s *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp32, a->fj);
    gen_helper_fp_tintrz_l_s(fp64, cpu_env, fp32);
    gen_store_fpr64(fp64, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftintrz_l_d(DisasContext *ctx, arg_ftintrz_l_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_tintrz_l_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_ftintrne_w_s(DisasContext *ctx, arg_ftintrne_w_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_tintrne_w_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_ftintrne_w_d(DisasContext *ctx, arg_ftintrne_w_d *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp64, a->fj);
    gen_helper_fp_tintrne_w_d(fp32, cpu_env, fp64);
    gen_store_fpr32(fp32, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftintrne_l_s(DisasContext *ctx, arg_ftintrne_l_s *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp32, a->fj);
    gen_helper_fp_tintrne_l_s(fp64, cpu_env, fp32);
    gen_store_fpr64(fp64, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftintrne_l_d(DisasContext *ctx, arg_ftintrne_l_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_tintrne_l_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_ftint_w_s(DisasContext *ctx, arg_ftint_w_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_tint_w_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_ftint_w_d(DisasContext *ctx, arg_ftint_w_d *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp64, a->fj);
    gen_helper_fp_tint_w_d(fp32, cpu_env, fp64);
    gen_store_fpr32(fp32, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftint_l_s(DisasContext *ctx, arg_ftint_l_s *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp32, a->fj);
    gen_helper_fp_tint_l_s(fp64, cpu_env, fp32);
    gen_store_fpr64(fp64, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ftint_l_d(DisasContext *ctx, arg_ftint_l_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_tint_l_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_ffint_s_w(DisasContext *ctx, arg_ffint_s_w *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_fint_s_w(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_ffint_s_l(DisasContext *ctx, arg_ffint_s_l *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp64, a->fj);
    gen_helper_fp_fint_s_l(fp32, cpu_env, fp64);
    gen_store_fpr32(fp32, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ffint_d_w(DisasContext *ctx, arg_ffint_d_w *a)
{
    TCGv_i32 fp32 = tcg_temp_new_i32();
    TCGv_i64 fp64 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp32, a->fj);
    gen_helper_fp_fint_d_w(fp64, cpu_env, fp32);
    gen_store_fpr64(fp64, a->fd);

    tcg_temp_free_i32(fp32);
    tcg_temp_free_i64(fp64);

    return true;
}

static bool trans_ffint_d_l(DisasContext *ctx, arg_ffint_d_l *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_fint_d_l(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}

static bool trans_frint_s(DisasContext *ctx, arg_frint_s *a)
{
    TCGv_i32 fp0;

    fp0 = tcg_temp_new_i32();

    check_fpu_enabled(ctx);
    gen_load_fpr32(fp0, a->fj);
    gen_helper_fp_rint_s(fp0, cpu_env, fp0);
    gen_store_fpr32(fp0, a->fd);

    tcg_temp_free_i32(fp0);

    return true;
}

static bool trans_frint_d(DisasContext *ctx, arg_frint_d *a)
{
    TCGv_i64 fp0;

    fp0 = tcg_temp_new_i64();

    check_fpu_enabled(ctx);
    gen_load_fpr64(fp0, a->fj);
    gen_helper_fp_rint_d(fp0, cpu_env, fp0);
    gen_store_fpr64(fp0, a->fd);

    tcg_temp_free_i64(fp0);

    return true;
}
