/*
 * QEMU LoongArch CPU
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

DEF_HELPER_2(raise_exception, noreturn, env, i32)

DEF_HELPER_FLAGS_1(bitrev_w, TCG_CALL_NO_RWG_SE, tl, tl)
DEF_HELPER_FLAGS_1(bitrev_d, TCG_CALL_NO_RWG_SE, tl, tl)
DEF_HELPER_FLAGS_1(bitswap, TCG_CALL_NO_RWG_SE, tl, tl)

DEF_HELPER_3(asrtle_d, void, env, tl, tl)
DEF_HELPER_3(asrtgt_d, void, env, tl, tl)

DEF_HELPER_3(crc32, tl, tl, tl, tl)
DEF_HELPER_3(crc32c, tl, tl, tl, tl)
DEF_HELPER_2(cpucfg, tl, env, tl)

/* Floating-point helper */
DEF_HELPER_3(fadd_s, i64, env, i64, i64)
DEF_HELPER_3(fadd_d, i64, env, i64, i64)
DEF_HELPER_3(fsub_s, i64, env, i64, i64)
DEF_HELPER_3(fsub_d, i64, env, i64, i64)
DEF_HELPER_3(fmul_s, i64, env, i64, i64)
DEF_HELPER_3(fmul_d, i64, env, i64, i64)
DEF_HELPER_3(fdiv_s, i64, env, i64, i64)
DEF_HELPER_3(fdiv_d, i64, env, i64, i64)
DEF_HELPER_3(fmax_s, i64, env, i64, i64)
DEF_HELPER_3(fmax_d, i64, env, i64, i64)
DEF_HELPER_3(fmin_s, i64, env, i64, i64)
DEF_HELPER_3(fmin_d, i64, env, i64, i64)
DEF_HELPER_3(fmaxa_s, i64, env, i64, i64)
DEF_HELPER_3(fmaxa_d, i64, env, i64, i64)
DEF_HELPER_3(fmina_s, i64, env, i64, i64)
DEF_HELPER_3(fmina_d, i64, env, i64, i64)

DEF_HELPER_5(fmuladd_s, i64, env, i64, i64, i64, i32)
DEF_HELPER_5(fmuladd_d, i64, env, i64, i64, i64, i32)

DEF_HELPER_3(fscaleb_s, i64, env, i64, i64)
DEF_HELPER_3(fscaleb_d, i64, env, i64, i64)

DEF_HELPER_2(flogb_s, i64, env, i64)
DEF_HELPER_2(flogb_d, i64, env, i64)

DEF_HELPER_2(fsqrt_s, i64, env, i64)
DEF_HELPER_2(fsqrt_d, i64, env, i64)
DEF_HELPER_2(frsqrt_s, i64, env, i64)
DEF_HELPER_2(frsqrt_d, i64, env, i64)
DEF_HELPER_2(frecip_s, i64, env, i64)
DEF_HELPER_2(frecip_d, i64, env, i64)

DEF_HELPER_FLAGS_2(fclass_s, TCG_CALL_NO_RWG_SE, i64, env, i64)
DEF_HELPER_FLAGS_2(fclass_d, TCG_CALL_NO_RWG_SE, i64, env, i64)
