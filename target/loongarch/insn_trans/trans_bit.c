/*
 * LoongArch translate functions
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

static bool gen_r2(DisasContext *ctx, arg_fmt_rdrj *a,
                   DisasExtend src_ext, DisasExtend dst_ext,
                   void (*func)(TCGv, TCGv))
{
    TCGv dest = gpr_dst(ctx, a->rd, dst_ext);
    TCGv src1 = gpr_src(ctx, a->rj, src_ext);

    func(dest, src1);

    if (dst_ext) {
        gen_set_gpr(a->rd, dest, dst_ext);
    }
    return true;
}

static bool trans_bytepick_w(DisasContext *ctx, arg_bytepick_w *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rk, EXT_NONE);

    tcg_gen_concat_tl_i64(dest, src1, src2);
    tcg_gen_sextract_i64(dest, dest, (32 - (a->sa2) * 8), 32);

    return true;
}

static bool trans_bytepick_d(DisasContext *ctx, arg_bytepick_d *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);
    TCGv src2 = gpr_src(ctx, a->rk, EXT_NONE);

    tcg_gen_extract2_i64(dest, src1, src2, (64 - (a->sa3) * 8));
    return true;
}

static bool trans_bstrins_w(DisasContext *ctx, arg_bstrins_w *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);

    if (a->lsbw > a->msbw) {
        return false;
    }

    tcg_gen_deposit_tl(dest, dest, src1, a->lsbw, a->msbw - a->lsbw + 1);
    tcg_gen_ext32s_tl(dest, dest);

    return true;
}

static bool trans_bstrins_d(DisasContext *ctx, arg_bstrins_d *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);

    if (a->lsbd > a->msbd) {
        return false;
    }

    tcg_gen_deposit_tl(dest, dest, src1, a->lsbd, a->msbd - a->lsbd + 1);
    return true;
}

static bool trans_bstrpick_w(DisasContext *ctx, arg_bstrpick_w *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);

    if (a->lsbw > a->msbw) {
        return false;
    }

    tcg_gen_extract_tl(dest, src1, a->lsbw, a->msbw - a->lsbw + 1);
    tcg_gen_ext32s_tl(dest, dest);
    return true;
}

static bool trans_bstrpick_d(DisasContext *ctx, arg_bstrpick_d *a)
{
    TCGv dest = gpr_dst(ctx, a->rd, EXT_NONE);
    TCGv src1 = gpr_src(ctx, a->rj, EXT_NONE);

    if (a->lsbd > a->msbd) {
        return false;
    }

    tcg_gen_extract_tl(dest, src1, a->lsbd, a->msbd - a->lsbd + 1);
    return true;
}

static void gen_clz_w(TCGv dest, TCGv src1)
{
    tcg_gen_clzi_tl(dest, src1, TARGET_LONG_BITS);
    tcg_gen_subi_tl(dest, dest, TARGET_LONG_BITS - 32);
}

static void gen_clo_w(TCGv dest, TCGv src1)
{
    tcg_gen_not_tl(dest, src1);
    tcg_gen_ext32u_tl(dest, dest);
    gen_clz_w(dest, dest);
}

static void gen_ctz_w(TCGv dest, TCGv src1)
{
    tcg_gen_ori_tl(dest, src1, (target_ulong)MAKE_64BIT_MASK(32, 32));
    tcg_gen_ctzi_tl(dest, dest, 32);
}

static void gen_cto_w(TCGv dest, TCGv src1)
{
    tcg_gen_not_tl(dest, src1);
    tcg_gen_ext32u_tl(dest, dest);
    gen_ctz_w(dest, dest);
}

static void gen_clz_d(TCGv dest, TCGv src1)
{
    tcg_gen_clzi_i64(dest, src1, TARGET_LONG_BITS);
}

static void gen_clo_d(TCGv dest, TCGv src1)
{
    tcg_gen_not_tl(dest, src1);
    gen_clz_d(dest, dest);
}

static void gen_ctz_d(TCGv dest, TCGv src1)
{
    tcg_gen_ctzi_tl(dest, src1, TARGET_LONG_BITS);
}

static void gen_cto_d(TCGv dest, TCGv src1)
{
    tcg_gen_not_tl(dest, src1);
    gen_ctz_d(dest, dest);
}

static void gen_revb_2w(TCGv dest, TCGv src1)
{
    tcg_gen_bswap64_i64(dest, src1);
    tcg_gen_rotri_i64(dest, dest, 32);
}

static void gen_revb_2h(TCGv dest, TCGv src1)
{
    TCGv mask = tcg_constant_tl(0x00FF00FF);
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();

    tcg_gen_shri_tl(t0, src1, 8);
    tcg_gen_and_tl(t0, t0, mask);
    tcg_gen_and_tl(t1, src1, mask);
    tcg_gen_shli_tl(t1, t1, 8);
    tcg_gen_or_tl(dest, t0, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
}

static void gen_revb_4h(TCGv dest, TCGv src1)
{
    TCGv mask = tcg_constant_tl(0x00FF00FF00FF00FFULL);
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();

    tcg_gen_shri_tl(t0, src1, 8);
    tcg_gen_and_tl(t0, t0, mask);
    tcg_gen_and_tl(t1, src1, mask);
    tcg_gen_shli_tl(t1, t1, 8);
    tcg_gen_or_tl(dest, t0, t1);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
}

static void gen_revh_2w(TCGv dest, TCGv src1)
{
    TCGv_i64 t0 = tcg_temp_new_i64();
    TCGv_i64 t1 = tcg_temp_new_i64();
    TCGv_i64 mask = tcg_constant_i64(0x0000ffff0000ffffull);

    tcg_gen_shri_i64(t0, src1, 16);
    tcg_gen_and_i64(t1, src1, mask);
    tcg_gen_and_i64(t0, t0, mask);
    tcg_gen_shli_i64(t1, t1, 16);
    tcg_gen_or_i64(dest, t1, t0);

    tcg_temp_free_i64(t0);
    tcg_temp_free_i64(t1);
}

static void gen_revh_d(TCGv dest, TCGv src1)
{
    TCGv t0 = tcg_temp_new();
    TCGv t1 = tcg_temp_new();
    TCGv mask = tcg_constant_tl(0x0000FFFF0000FFFFULL);

    tcg_gen_shri_tl(t1, src1, 16);
    tcg_gen_and_tl(t1, t1, mask);
    tcg_gen_and_tl(t0, src1, mask);
    tcg_gen_shli_tl(t0, t0, 16);
    tcg_gen_or_tl(t0, t0, t1);
    tcg_gen_rotri_tl(dest, t0, 32);

    tcg_temp_free(t0);
    tcg_temp_free(t1);
}

static void gen_maskeqz(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv zero = tcg_constant_tl(0);

    tcg_gen_movcond_tl(TCG_COND_EQ, dest, src2, zero, zero, src1);
}

static void gen_masknez(TCGv dest, TCGv src1, TCGv src2)
{
    TCGv zero = tcg_constant_tl(0);

    tcg_gen_movcond_tl(TCG_COND_NE, dest, src2, zero, zero, src1);
}

TRANS(ext_w_h, gen_r2, EXT_NONE, EXT_NONE, tcg_gen_ext16s_tl)
TRANS(ext_w_b, gen_r2, EXT_NONE, EXT_NONE, tcg_gen_ext8s_tl)
TRANS(clo_w, gen_r2, EXT_NONE, EXT_NONE, gen_clo_w)
TRANS(clz_w, gen_r2, EXT_ZERO, EXT_NONE, gen_clz_w)
TRANS(cto_w, gen_r2, EXT_NONE, EXT_NONE, gen_cto_w)
TRANS(ctz_w, gen_r2, EXT_NONE, EXT_NONE, gen_ctz_w)
TRANS(clo_d, gen_r2, EXT_NONE, EXT_NONE, gen_clo_d)
TRANS(clz_d, gen_r2, EXT_NONE, EXT_NONE, gen_clz_d)
TRANS(cto_d, gen_r2, EXT_NONE, EXT_NONE, gen_cto_d)
TRANS(ctz_d, gen_r2, EXT_NONE, EXT_NONE, gen_ctz_d)
TRANS(revb_2h, gen_r2, EXT_NONE, EXT_SIGN, gen_revb_2h)
TRANS(revb_4h, gen_r2, EXT_NONE, EXT_NONE, gen_revb_4h)
TRANS(revb_2w, gen_r2, EXT_NONE, EXT_NONE, gen_revb_2w)
TRANS(revb_d, gen_r2, EXT_NONE, EXT_NONE, tcg_gen_bswap64_i64)
TRANS(revh_2w, gen_r2, EXT_NONE, EXT_NONE, gen_revh_2w)
TRANS(revh_d, gen_r2, EXT_NONE, EXT_NONE, gen_revh_d)
TRANS(bitrev_4b, gen_r2, EXT_ZERO, EXT_SIGN, gen_helper_bitswap)
TRANS(bitrev_8b, gen_r2, EXT_NONE, EXT_NONE, gen_helper_bitswap)
TRANS(bitrev_w, gen_r2, EXT_NONE, EXT_SIGN, gen_helper_bitrev_w)
TRANS(bitrev_d, gen_r2, EXT_NONE, EXT_NONE, gen_helper_bitrev_d)
TRANS(maskeqz, gen_r3, EXT_NONE, EXT_NONE, EXT_NONE, gen_maskeqz)
TRANS(masknez, gen_r3, EXT_NONE, EXT_NONE, EXT_NONE, gen_masknez)
