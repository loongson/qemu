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

/* fcmp.cXXX.s */
DEF_HELPER_4(fcmp_c_s, i64, env, i64, i64, i32)
/* fcmp.sXXX.s */
DEF_HELPER_4(fcmp_s_s, i64, env, i64, i64, i32)
/* fcmp.cXXX.d */
DEF_HELPER_4(fcmp_c_d, i64, env, i64, i64, i32)
/* fcmp.sXXX.d */
DEF_HELPER_4(fcmp_s_d, i64, env, i64, i64, i32)

DEF_HELPER_2(fcvt_d_s, i64, env, i64)
DEF_HELPER_2(fcvt_s_d, i64, env, i64)
DEF_HELPER_2(ffint_d_w, i64, env, i64)
DEF_HELPER_2(ffint_d_l, i64, env, i64)
DEF_HELPER_2(ffint_s_w, i64, env, i64)
DEF_HELPER_2(ffint_s_l, i64, env, i64)
DEF_HELPER_2(ftintrm_l_s, i64, env, i64)
DEF_HELPER_2(ftintrm_l_d, i64, env, i64)
DEF_HELPER_2(ftintrm_w_s, i64, env, i64)
DEF_HELPER_2(ftintrm_w_d, i64, env, i64)
DEF_HELPER_2(ftintrp_l_s, i64, env, i64)
DEF_HELPER_2(ftintrp_l_d, i64, env, i64)
DEF_HELPER_2(ftintrp_w_s, i64, env, i64)
DEF_HELPER_2(ftintrp_w_d, i64, env, i64)
DEF_HELPER_2(ftintrz_l_s, i64, env, i64)
DEF_HELPER_2(ftintrz_l_d, i64, env, i64)
DEF_HELPER_2(ftintrz_w_s, i64, env, i64)
DEF_HELPER_2(ftintrz_w_d, i64, env, i64)
DEF_HELPER_2(ftintrne_l_s, i64, env, i64)
DEF_HELPER_2(ftintrne_l_d, i64, env, i64)
DEF_HELPER_2(ftintrne_w_s, i64, env, i64)
DEF_HELPER_2(ftintrne_w_d, i64, env, i64)
DEF_HELPER_2(ftint_l_s, i64, env, i64)
DEF_HELPER_2(ftint_l_d, i64, env, i64)
DEF_HELPER_2(ftint_w_s, i64, env, i64)
DEF_HELPER_2(ftint_w_d, i64, env, i64)
DEF_HELPER_2(frint_s, i64, env, i64)
DEF_HELPER_2(frint_d, i64, env, i64)

DEF_HELPER_FLAGS_2(set_rounding_mode, TCG_CALL_NO_RWG, void, env, i32)

/*Core functions */
#ifndef CONFIG_USER_ONLY
DEF_HELPER_2(csr_rdq, i64, env, i64)
DEF_HELPER_3(csr_wrq, i64, env, tl, i64)
DEF_HELPER_4(csr_xchgq, i64, env, tl, tl, i64)

DEF_HELPER_1(tlbwr, void, env)
DEF_HELPER_1(tlbfill, void, env)
DEF_HELPER_1(tlbsrch, void, env)
DEF_HELPER_1(tlbrd, void, env)
DEF_HELPER_1(tlbclr, void, env)
DEF_HELPER_1(tlbflush, void, env)
DEF_HELPER_4(invtlb, void, env, tl, tl, tl)
DEF_HELPER_2(iocsr_read, i64, env, tl)
DEF_HELPER_4(iocsr_write, void, env, tl, tl, i32)
DEF_HELPER_4(lddir, tl, env, tl, tl, i32)
DEF_HELPER_4(ldpte, void, env, tl, tl, i32)
DEF_HELPER_1(ertn, void, env)
DEF_HELPER_1(idle, void, env)
DEF_HELPER_1(rdtime_d, i64, env)
#endif /* !CONFIG_USER_ONLY */
