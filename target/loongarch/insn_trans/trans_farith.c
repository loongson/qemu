/*
 * LoongArch translate functions
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

static bool gen_f3(DisasContext *ctx, arg_fmt_fdfjfk *a,
                   void (*func)(TCGv, TCGv_env, TCGv, TCGv))
{
    func(cpu_fpr[a->fd], cpu_env, cpu_fpr[a->fj], cpu_fpr[a->fk]);
    return true;
}

static bool gen_f2(DisasContext *ctx, arg_fmt_fdfj *a,
                   void (*func)(TCGv, TCGv_env, TCGv))
{
    func(cpu_fpr[a->fd], cpu_env, cpu_fpr[a->fj]);
    return true;
}

static bool gen_muladd(DisasContext *ctx, arg_fmt_fdfjfkfa *a,
                       void (*func)(TCGv, TCGv_env, TCGv, TCGv, TCGv, TCGv_i32),
                       int flag)
{
    TCGv_i32 tflag = tcg_constant_i32(flag);
    func(cpu_fpr[a->fd], cpu_env, cpu_fpr[a->fj],
         cpu_fpr[a->fk], cpu_fpr[a->fa], tflag);
    return true;
}

static bool trans_fcopysign_s(DisasContext *ctx, arg_fmt_fdfjfk *a)
{
    tcg_gen_deposit_i64(cpu_fpr[a->fd], cpu_fpr[a->fk], cpu_fpr[a->fj], 0, 31);
    return true;
}

static bool trans_fcopysign_d(DisasContext *ctx, arg_fmt_fdfjfk *a)
{
    tcg_gen_deposit_i64(cpu_fpr[a->fd], cpu_fpr[a->fk], cpu_fpr[a->fj], 0, 63);
    return true;
}

static bool trans_fmax_s(DisasContext *ctx, arg_fmt_fdfjfk *a)
{
    tcg_gen_umax_i64(cpu_fpr[a->fd], cpu_fpr[a->fj], cpu_fpr[a->fk]);
    gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    return true;
}

static bool trans_fmax_d(DisasContext *ctx, arg_fmt_fdfjfk *a)
{
    tcg_gen_umax_i64(cpu_fpr[a->fd], cpu_fpr[a->fj], cpu_fpr[a->fk]);
    return true;
}

static bool trans_fmin_s(DisasContext *ctx, arg_fmt_fdfjfk *a)
{
    tcg_gen_umin_i64(cpu_fpr[a->fd], cpu_fpr[a->fj], cpu_fpr[a->fk]);
    gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    return true;
}

static bool trans_fmin_d(DisasContext *ctx, arg_fmt_fdfjfk *a)
{
    tcg_gen_umin_i64(cpu_fpr[a->fd], cpu_fpr[a->fj], cpu_fpr[a->fk]);
    return true;
}

static bool trans_fabs_s(DisasContext *ctx, arg_fmt_fdfj *a)
{
    tcg_gen_ext32u_tl(cpu_fpr[a->fj], cpu_fpr[a->fj]);
    tcg_gen_abs_i64(cpu_fpr[a->fd], cpu_fpr[a->fj]);
    gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    return true;
}

static bool trans_fabs_d(DisasContext *ctx, arg_fmt_fdfj *a)
{
    tcg_gen_abs_i64(cpu_fpr[a->fd], cpu_fpr[a->fj]);
    return true;
}

static bool trans_fneg_s(DisasContext *ctx, arg_fmt_fdfj *a)
{
    tcg_gen_xori_i64(cpu_fpr[a->fd], cpu_fpr[a->fj], 0x80000000);
    gen_nanbox_s(cpu_fpr[a->fd], cpu_fpr[a->fd]);
    return true;
}

static bool trans_fneg_d(DisasContext *ctx, arg_fmt_fdfj *a)
{
    tcg_gen_xori_i64(cpu_fpr[a->fd], cpu_fpr[a->fj], 0x8000000000000000LL);
    return true;
}

TRANS(fadd_s, gen_f3, gen_helper_fadd_s)
TRANS(fadd_d, gen_f3, gen_helper_fadd_d)
TRANS(fsub_s, gen_f3, gen_helper_fsub_s)
TRANS(fsub_d, gen_f3, gen_helper_fsub_d)
TRANS(fmul_s, gen_f3, gen_helper_fmul_s)
TRANS(fmul_d, gen_f3, gen_helper_fmul_d)
TRANS(fdiv_s, gen_f3, gen_helper_fdiv_s)
TRANS(fdiv_d, gen_f3, gen_helper_fdiv_d)
TRANS(fmaxa_s, gen_f3, gen_helper_fmaxa_s)
TRANS(fmaxa_d, gen_f3, gen_helper_fmaxa_d)
TRANS(fmina_s, gen_f3, gen_helper_fmina_s)
TRANS(fmina_d, gen_f3, gen_helper_fmina_d)
TRANS(fscaleb_s, gen_f3, gen_helper_fscaleb_s)
TRANS(fscaleb_d, gen_f3, gen_helper_fscaleb_d)
TRANS(fsqrt_s, gen_f2, gen_helper_fsqrt_s)
TRANS(fsqrt_d, gen_f2, gen_helper_fsqrt_d)
TRANS(frecip_s, gen_f2, gen_helper_frecip_s)
TRANS(frecip_d, gen_f2, gen_helper_frecip_d)
TRANS(frsqrt_s, gen_f2, gen_helper_frsqrt_s)
TRANS(frsqrt_d, gen_f2, gen_helper_frsqrt_d)
TRANS(flogb_s, gen_f2, gen_helper_flogb_s)
TRANS(flogb_d, gen_f2, gen_helper_flogb_d)
TRANS(fclass_s, gen_f2, gen_helper_fclass_s)
TRANS(fclass_d, gen_f2, gen_helper_fclass_d)
TRANS(fmadd_s, gen_muladd, gen_helper_fmuladd_s, 0)
TRANS(fmadd_d, gen_muladd, gen_helper_fmuladd_d, 0)
TRANS(fmsub_s, gen_muladd, gen_helper_fmuladd_s, float_muladd_negate_c)
TRANS(fmsub_d, gen_muladd, gen_helper_fmuladd_d, float_muladd_negate_c)
TRANS(fnmadd_s, gen_muladd, gen_helper_fmuladd_s,
      float_muladd_negate_product | float_muladd_negate_c)
TRANS(fnmadd_d, gen_muladd, gen_helper_fmuladd_d,
      float_muladd_negate_product | float_muladd_negate_c)
TRANS(fnmsub_s, gen_muladd, gen_helper_fmuladd_s, float_muladd_negate_product)
TRANS(fnmsub_d, gen_muladd, gen_helper_fmuladd_d, float_muladd_negate_product)
