/*
 * QEMU LoongArch CPU
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

DEF_HELPER_3(raise_exception_err, noreturn, env, i32, int)
DEF_HELPER_2(raise_exception, noreturn, env, i32)

DEF_HELPER_2(cto_w, tl, env, tl)
DEF_HELPER_2(ctz_w, tl, env, tl)
DEF_HELPER_2(cto_d, tl, env, tl)
DEF_HELPER_2(ctz_d, tl, env, tl)
DEF_HELPER_2(bitrev_w, tl, env, tl)
DEF_HELPER_2(bitrev_d, tl, env, tl)

DEF_HELPER_FLAGS_1(loongarch_bitswap, TCG_CALL_NO_RWG_SE, tl, tl)
DEF_HELPER_FLAGS_1(loongarch_dbitswap, TCG_CALL_NO_RWG_SE, tl, tl)

DEF_HELPER_3(asrtle_d, void, env, tl, tl)
DEF_HELPER_3(asrtgt_d, void, env, tl, tl)

DEF_HELPER_3(crc32, tl, tl, tl, i32)
DEF_HELPER_3(crc32c, tl, tl, tl, i32)
DEF_HELPER_2(cpucfg, tl, env, tl)

/* Floating-point helper */
DEF_HELPER_3(fp_add_s, i32, env, i32, i32)
DEF_HELPER_3(fp_add_d, i64, env, i64, i64)
DEF_HELPER_3(fp_sub_s, i32, env, i32, i32)
DEF_HELPER_3(fp_sub_d, i64, env, i64, i64)
DEF_HELPER_3(fp_mul_s, i32, env, i32, i32)
DEF_HELPER_3(fp_mul_d, i64, env, i64, i64)
DEF_HELPER_3(fp_div_s, i32, env, i32, i32)
DEF_HELPER_3(fp_div_d, i64, env, i64, i64)
DEF_HELPER_3(fp_max_s, i32, env, i32, i32)
DEF_HELPER_3(fp_max_d, i64, env, i64, i64)
DEF_HELPER_3(fp_maxa_s, i32, env, i32, i32)
DEF_HELPER_3(fp_maxa_d, i64, env, i64, i64)
DEF_HELPER_3(fp_min_s, i32, env, i32, i32)
DEF_HELPER_3(fp_min_d, i64, env, i64, i64)
DEF_HELPER_3(fp_mina_s, i32, env, i32, i32)
DEF_HELPER_3(fp_mina_d, i64, env, i64, i64)

DEF_HELPER_4(fp_madd_s, i32, env, i32, i32, i32)
DEF_HELPER_4(fp_madd_d, i64, env, i64, i64, i64)
DEF_HELPER_4(fp_msub_s, i32, env, i32, i32, i32)
DEF_HELPER_4(fp_msub_d, i64, env, i64, i64, i64)
DEF_HELPER_4(fp_nmadd_s, i32, env, i32, i32, i32)
DEF_HELPER_4(fp_nmadd_d, i64, env, i64, i64, i64)
DEF_HELPER_4(fp_nmsub_s, i32, env, i32, i32, i32)
DEF_HELPER_4(fp_nmsub_d, i64, env, i64, i64, i64)

DEF_HELPER_3(fp_exp2_s, i32, env, i32, i32)
DEF_HELPER_3(fp_exp2_d, i64, env, i64, i64)
DEF_HELPER_2(fp_logb_s, i32, env, i32)
DEF_HELPER_2(fp_logb_d, i64, env, i64)

DEF_HELPER_1(fp_abs_s, i32, i32)
DEF_HELPER_1(fp_abs_d, i64, i64)
DEF_HELPER_1(fp_neg_s, i32, i32)
DEF_HELPER_1(fp_neg_d, i64, i64)

DEF_HELPER_2(fp_sqrt_s, i32, env, i32)
DEF_HELPER_2(fp_sqrt_d, i64, env, i64)
DEF_HELPER_2(fp_rsqrt_s, i32, env, i32)
DEF_HELPER_2(fp_rsqrt_d, i64, env, i64)
DEF_HELPER_2(fp_recip_s, i32, env, i32)
DEF_HELPER_2(fp_recip_d, i64, env, i64)

DEF_HELPER_FLAGS_2(fp_class_s, TCG_CALL_NO_RWG_SE, i32, env, i32)
DEF_HELPER_FLAGS_2(fp_class_d, TCG_CALL_NO_RWG_SE, i64, env, i64)
