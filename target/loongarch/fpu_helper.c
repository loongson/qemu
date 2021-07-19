/*
 * LoongArch float point emulation helpers for qemu
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "cpu.h"
#include "fpu_helper.h"
#include "exec/helper-proto.h"
#include "exec/exec-all.h"
#include "exec/cpu_ldst.h"
#include "fpu/softfloat.h"

#define FP_TO_INT32_OVERFLOW 0x7fffffff
#define FP_TO_INT64_OVERFLOW 0x7fffffffffffffffULL

#define FP_CLASS_SIGNALING_NAN      0x001
#define FP_CLASS_QUIET_NAN          0x002
#define FP_CLASS_NEGATIVE_INFINITY  0x004
#define FP_CLASS_NEGATIVE_NORMAL    0x008
#define FP_CLASS_NEGATIVE_SUBNORMAL 0x010
#define FP_CLASS_NEGATIVE_ZERO      0x020
#define FP_CLASS_POSITIVE_INFINITY  0x040
#define FP_CLASS_POSITIVE_NORMAL    0x080
#define FP_CLASS_POSITIVE_SUBNORMAL 0x100
#define FP_CLASS_POSITIVE_ZERO      0x200

/* convert loongarch rounding mode in fcsr0 to IEEE library */
const FloatRoundMode ieee_rm[4] = {
    float_round_nearest_even,
    float_round_to_zero,
    float_round_up,
    float_round_down
};

int ieee_ex_to_loongarch(int xcpt)
{
    int ret = 0;
    if (xcpt) {
        if (xcpt & float_flag_invalid) {
            ret |= FP_INVALID;
        }
        if (xcpt & float_flag_overflow) {
            ret |= FP_OVERFLOW;
        }
        if (xcpt & float_flag_underflow) {
            ret |= FP_UNDERFLOW;
        }
        if (xcpt & float_flag_divbyzero) {
            ret |= FP_DIV0;
        }
        if (xcpt & float_flag_inexact) {
            ret |= FP_INEXACT;
        }
    }
    return ret;
}

static inline void update_fcsr0(CPULoongArchState *env, uintptr_t pc)
{
    int tmp = ieee_ex_to_loongarch(get_float_exception_flags(
                                  &env->active_fpu.fp_status));

    SET_FP_CAUSE(env->active_fpu.fcsr0, tmp);
    if (tmp) {
        set_float_exception_flags(0, &env->active_fpu.fp_status);

        if (GET_FP_ENABLE(env->active_fpu.fcsr0) & tmp) {
            do_raise_exception(env, EXCP_FPE, pc);
        } else {
            UPDATE_FP_FLAGS(env->active_fpu.fcsr0, tmp);
        }
    }
}

uint64_t helper_fp_sqrt_d(CPULoongArchState *env, uint64_t fp)
{
    fp = float64_sqrt(fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp;
}

uint32_t helper_fp_sqrt_s(CPULoongArchState *env, uint32_t fp)
{
    fp = float32_sqrt(fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp;
}

uint64_t helper_fp_abs_d(uint64_t fp)
{
    return float64_abs(fp);
}
uint32_t helper_fp_abs_s(uint32_t fp)
{
    return float32_abs(fp);
}

uint64_t helper_fp_neg_d(uint64_t fp)
{
    return float64_chs(fp);
}
uint32_t helper_fp_neg_s(uint32_t fp)
{
    return float32_chs(fp);
}

uint64_t helper_fp_recip_d(CPULoongArchState *env, uint64_t fp)
{
    uint64_t fp1;

    fp1 = float64_div(float64_one, fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp1;
}

uint32_t helper_fp_recip_s(CPULoongArchState *env, uint32_t fp)
{
    uint32_t fp1;

    fp1 = float32_div(float32_one, fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp1;
}

uint64_t helper_fp_rsqrt_d(CPULoongArchState *env, uint64_t fp)
{
    uint64_t fp1;

    fp1 = float64_sqrt(fp, &env->active_fpu.fp_status);
    fp1 = float64_div(float64_one, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp1;
}

uint32_t helper_fp_rsqrt_s(CPULoongArchState *env, uint32_t fp)
{
    uint32_t fp1;

    fp1 = float32_sqrt(fp, &env->active_fpu.fp_status);
    fp1 = float32_div(float32_one, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp1;
}

uint32_t fp_class_s(uint32_t arg, float_status *status)
{
    if (float32_is_signaling_nan(arg, status)) {
        return FP_CLASS_SIGNALING_NAN;
    } else if (float32_is_quiet_nan(arg, status)) {
        return FP_CLASS_QUIET_NAN;
    } else if (float32_is_neg(arg)) {
        if (float32_is_infinity(arg)) {
            return FP_CLASS_NEGATIVE_INFINITY;
        } else if (float32_is_zero(arg)) {
            return FP_CLASS_NEGATIVE_ZERO;
        } else if (float32_is_zero_or_denormal(arg)) {
            return FP_CLASS_NEGATIVE_SUBNORMAL;
        } else {
            return FP_CLASS_NEGATIVE_NORMAL;
        }
    } else {
        if (float32_is_infinity(arg)) {
            return FP_CLASS_POSITIVE_INFINITY;
        } else if (float32_is_zero(arg)) {
            return FP_CLASS_POSITIVE_ZERO;
        } else if (float32_is_zero_or_denormal(arg)) {
            return FP_CLASS_POSITIVE_SUBNORMAL;
        } else {
            return FP_CLASS_POSITIVE_NORMAL;
        }
    }
}

uint32_t helper_fp_class_s(CPULoongArchState *env, uint32_t arg)
{
    return fp_class_s(arg, &env->active_fpu.fp_status);
}

uint64_t fp_class_d(uint64_t arg, float_status *status)
{
    if (float64_is_signaling_nan(arg, status)) {
        return FP_CLASS_SIGNALING_NAN;
    } else if (float64_is_quiet_nan(arg, status)) {
        return FP_CLASS_QUIET_NAN;
    } else if (float64_is_neg(arg)) {
        if (float64_is_infinity(arg)) {
            return FP_CLASS_NEGATIVE_INFINITY;
        } else if (float64_is_zero(arg)) {
            return FP_CLASS_NEGATIVE_ZERO;
        } else if (float64_is_zero_or_denormal(arg)) {
            return FP_CLASS_NEGATIVE_SUBNORMAL;
        } else {
            return FP_CLASS_NEGATIVE_NORMAL;
        }
    } else {
        if (float64_is_infinity(arg)) {
            return FP_CLASS_POSITIVE_INFINITY;
        } else if (float64_is_zero(arg)) {
            return FP_CLASS_POSITIVE_ZERO;
        } else if (float64_is_zero_or_denormal(arg)) {
            return FP_CLASS_POSITIVE_SUBNORMAL;
        } else {
            return FP_CLASS_POSITIVE_NORMAL;
        }
    }
}

uint64_t helper_fp_class_d(CPULoongArchState *env, uint64_t arg)
{
    return fp_class_d(arg, &env->active_fpu.fp_status);
}

uint64_t helper_fp_add_d(CPULoongArchState *env, uint64_t fp, uint64_t fp1)
{
    uint64_t fp2;

    fp2 = float64_add(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

uint32_t helper_fp_add_s(CPULoongArchState *env, uint32_t fp, uint32_t fp1)
{
    uint32_t fp2;

    fp2 = float32_add(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

uint64_t helper_fp_sub_d(CPULoongArchState *env, uint64_t fp, uint64_t fp1)
{
    uint64_t fp2;

    fp2 = float64_sub(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

uint32_t helper_fp_sub_s(CPULoongArchState *env, uint32_t fp, uint32_t fp1)
{
    uint32_t fp2;

    fp2 = float32_sub(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

uint64_t helper_fp_mul_d(CPULoongArchState *env, uint64_t fp, uint64_t fp1)
{
    uint64_t fp2;

    fp2 = float64_mul(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

uint32_t helper_fp_mul_s(CPULoongArchState *env, uint32_t fp, uint32_t fp1)
{
    uint32_t fp2;

    fp2 = float32_mul(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

uint64_t helper_fp_div_d(CPULoongArchState *env, uint64_t fp, uint64_t fp1)
{
    uint64_t fp2;

    fp2 = float64_div(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

uint32_t helper_fp_div_s(CPULoongArchState *env, uint32_t fp, uint32_t fp1)
{
    uint32_t fp2;

    fp2 = float32_div(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

uint64_t helper_fp_exp2_d(CPULoongArchState *env,
                          uint64_t fp, uint64_t fp1)
{
    uint64_t fp2;
    int64_t n = (int64_t)fp1;

    fp2 = float64_scalbn(fp,
                         n >  0x1000 ?  0x1000 :
                         n < -0x1000 ? -0x1000 : n,
                         &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

uint32_t helper_fp_exp2_s(CPULoongArchState *env,
                          uint32_t fp, uint32_t fp1)
{
    uint32_t fp2;
    int32_t n = (int32_t)fp1;

    fp2 = float32_scalbn(fp,
                         n >  0x200 ?  0x200 :
                         n < -0x200 ? -0x200 : n,
                         &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp2;
}

#define FP_MINMAX(name, bits, minmaxfunc)                                 \
uint ## bits ## _t helper_fp_ ## name(CPULoongArchState *env,             \
                                      uint ## bits ## _t fs,              \
                                      uint ## bits ## _t ft)              \
{                                                                         \
    uint ## bits ## _t fdret;                                             \
                                                                          \
    fdret = float ## bits ## _ ## minmaxfunc(fs, ft,                      \
                                             &env->active_fpu.fp_status); \
    update_fcsr0(env, GETPC());                                           \
    return fdret;                                                         \
}

FP_MINMAX(max_s, 32, maxnum)
FP_MINMAX(max_d, 64, maxnum)
FP_MINMAX(maxa_s, 32, maxnummag)
FP_MINMAX(maxa_d, 64, maxnummag)
FP_MINMAX(min_s, 32, minnum)
FP_MINMAX(min_d, 64, minnum)
FP_MINMAX(mina_s, 32, minnummag)
FP_MINMAX(mina_d, 64, minnummag)
#undef FP_MINMAX

#define FP_FMADDSUB(name, bits, muladd_arg)                       \
uint ## bits ## _t helper_fp_ ## name(CPULoongArchState *env,     \
                                      uint ## bits ## _t fs,      \
                                      uint ## bits ## _t ft,      \
                                      uint ## bits ## _t fd)      \
{                                                                 \
    uint ## bits ## _t fdret;                                     \
                                                                  \
    fdret = float ## bits ## _muladd(fs, ft, fd, muladd_arg,      \
                                     &env->active_fpu.fp_status); \
    update_fcsr0(env, GETPC());                                   \
    return fdret;                                                 \
}

FP_FMADDSUB(madd_s, 32, 0)
FP_FMADDSUB(madd_d, 64, 0)
FP_FMADDSUB(msub_s, 32, float_muladd_negate_c)
FP_FMADDSUB(msub_d, 64, float_muladd_negate_c)
FP_FMADDSUB(nmadd_s, 32, float_muladd_negate_result)
FP_FMADDSUB(nmadd_d, 64, float_muladd_negate_result)
FP_FMADDSUB(nmsub_s, 32, float_muladd_negate_result | float_muladd_negate_c)
FP_FMADDSUB(nmsub_d, 64, float_muladd_negate_result | float_muladd_negate_c)
#undef FP_FMADDSUB

uint32_t helper_fp_logb_s(CPULoongArchState *env, uint32_t fp)
{
    uint32_t fp1;

    fp1 = float32_log2(fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp1;
}

uint64_t helper_fp_logb_d(CPULoongArchState *env, uint64_t fp)
{
    uint64_t fp1;

    fp1 = float64_log2(fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return fp1;
}

void helper_movreg2cf_i32(CPULoongArchState *env, uint32_t cd, uint32_t src)
{
    env->active_fpu.cf[cd & 0x7] = src & 0x1;
}

void helper_movreg2cf_i64(CPULoongArchState *env, uint32_t cd, uint64_t src)
{
    env->active_fpu.cf[cd & 0x7] = src & 0x1;
}

/* fcmp.cond.s */
uint32_t helper_fp_cmp_caf_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = (float32_unordered_quiet(fp1, fp, &env->active_fpu.fp_status), 0);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_cun_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered_quiet(fp1, fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_ceq_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_eq_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_cueq_s(CPULoongArchState *env, uint32_t fp,
                              uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float32_eq_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_clt_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_lt_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_cult_s(CPULoongArchState *env, uint32_t fp,
                              uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float32_lt_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_cle_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_le_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_cule_s(CPULoongArchState *env, uint32_t fp,
                              uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float32_le_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_cne_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_lt_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float32_lt_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_cor_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_le_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float32_le_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_cune_s(CPULoongArchState *env, uint32_t fp,
                              uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float32_lt_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float32_lt_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}


uint32_t helper_fp_cmp_saf_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = (float32_unordered(fp1, fp, &env->active_fpu.fp_status), 0);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_sun_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered(fp1, fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_seq_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_eq(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_sueq_s(CPULoongArchState *env, uint32_t fp,
                              uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered(fp1, fp, &env->active_fpu.fp_status) ||
          float32_eq(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_slt_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_lt(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_sult_s(CPULoongArchState *env, uint32_t fp,
                              uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered(fp1, fp, &env->active_fpu.fp_status) ||
          float32_lt(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_sle_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_le(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_sule_s(CPULoongArchState *env, uint32_t fp,
                              uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered(fp1, fp, &env->active_fpu.fp_status) ||
          float32_le(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_sne_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_lt(fp1, fp, &env->active_fpu.fp_status) ||
          float32_lt(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_sor_s(CPULoongArchState *env, uint32_t fp,
                             uint32_t fp1)
{
    uint64_t ret;
    ret = float32_le(fp1, fp, &env->active_fpu.fp_status) ||
          float32_le(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint32_t helper_fp_cmp_sune_s(CPULoongArchState *env, uint32_t fp,
                              uint32_t fp1)
{
    uint64_t ret;
    ret = float32_unordered(fp1, fp, &env->active_fpu.fp_status) ||
          float32_lt(fp1, fp, &env->active_fpu.fp_status) ||
          float32_lt(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

/* fcmp.cond.d */
uint64_t helper_fp_cmp_caf_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = (float64_unordered_quiet(fp1, fp, &env->active_fpu.fp_status), 0);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_cun_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered_quiet(fp1, fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_ceq_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_eq_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_cueq_d(CPULoongArchState *env, uint64_t fp,
                              uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float64_eq_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_clt_d(CPULoongArchState *env, uint64_t fp,
                              uint64_t fp1)
{
    uint64_t ret;
    ret = float64_lt_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_cult_d(CPULoongArchState *env, uint64_t fp,
                              uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float64_lt_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_cle_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_le_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_cule_d(CPULoongArchState *env, uint64_t fp,
                              uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float64_le_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_cne_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_lt_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float64_lt_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_cor_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_le_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float64_le_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_cune_d(CPULoongArchState *env, uint64_t fp,
                              uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float64_lt_quiet(fp1, fp, &env->active_fpu.fp_status) ||
          float64_lt_quiet(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_saf_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = (float64_unordered(fp1, fp, &env->active_fpu.fp_status), 0);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_sun_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered(fp1, fp, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_seq_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_eq(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_sueq_d(CPULoongArchState *env, uint64_t fp,
                              uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered(fp1, fp, &env->active_fpu.fp_status) ||
          float64_eq(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_slt_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_lt(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_sult_d(CPULoongArchState *env, uint64_t fp,
                              uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered(fp1, fp, &env->active_fpu.fp_status) ||
          float64_lt(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_sle_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_le(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_sule_d(CPULoongArchState *env, uint64_t fp,
                              uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered(fp1, fp, &env->active_fpu.fp_status) ||
          float64_le(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_sne_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_lt(fp1, fp, &env->active_fpu.fp_status) ||
          float64_lt(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_sor_d(CPULoongArchState *env, uint64_t fp,
                             uint64_t fp1)
{
    uint64_t ret;
    ret = float64_le(fp1, fp, &env->active_fpu.fp_status) ||
          float64_le(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

uint64_t helper_fp_cmp_sune_d(CPULoongArchState *env, uint64_t fp,
                              uint64_t fp1)
{
    uint64_t ret;
    ret = float64_unordered(fp1, fp, &env->active_fpu.fp_status) ||
          float64_lt(fp1, fp, &env->active_fpu.fp_status) ||
          float64_lt(fp, fp1, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    if (ret) {
        return -1;
    } else {
        return 0;
    }
}

/* floating point conversion */
uint64_t helper_fp_cvt_d_s(CPULoongArchState *env, uint32_t src)
{
    uint64_t dest;

    dest = float32_to_float64(src, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_fint_d_w(CPULoongArchState *env, uint32_t src)
{
    uint64_t dest;

    dest = int32_to_float64(src, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_fint_d_l(CPULoongArchState *env, uint64_t src)
{
    uint64_t dest;

    dest = int64_to_float64(src, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_cvt_s_d(CPULoongArchState *env, uint64_t src)
{
    uint32_t dest;

    dest = float64_to_float32(src, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_fint_s_w(CPULoongArchState *env, uint32_t src)
{
    uint32_t dest;

    dest = int32_to_float32(src, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_fint_s_l(CPULoongArchState *env, uint64_t src)
{
    uint32_t dest;

    dest = int64_to_float32(src, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tintrm_l_d(CPULoongArchState *env, uint64_t src)
{
    uint64_t dest;

    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    dest = float64_to_int64(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tintrm_l_s(CPULoongArchState *env, uint32_t src)
{
    uint64_t dest;

    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    dest = float32_to_int64(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tintrm_w_d(CPULoongArchState *env, uint64_t src)
{
    uint32_t dest;

    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    dest = float64_to_int32(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tintrm_w_s(CPULoongArchState *env, uint32_t src)
{
    uint32_t dest;

    set_float_rounding_mode(float_round_down, &env->active_fpu.fp_status);
    dest = float32_to_int32(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tintrp_l_d(CPULoongArchState *env, uint64_t src)
{
    uint64_t dest;

    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    dest = float64_to_int64(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tintrp_l_s(CPULoongArchState *env, uint32_t src)
{
    uint64_t dest;

    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    dest = float32_to_int64(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tintrp_w_d(CPULoongArchState *env, uint64_t src)
{
    uint32_t dest;

    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    dest = float64_to_int32(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tintrp_w_s(CPULoongArchState *env, uint32_t src)
{
    uint32_t dest;

    set_float_rounding_mode(float_round_up, &env->active_fpu.fp_status);
    dest = float32_to_int32(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tintrz_l_d(CPULoongArchState *env, uint64_t src)
{
    uint64_t dest;

    dest = float64_to_int64_round_to_zero(src,
                                         &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tintrz_l_s(CPULoongArchState *env, uint32_t src)
{
    uint64_t dest;

    dest = float32_to_int64_round_to_zero(src, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tintrz_w_d(CPULoongArchState *env, uint64_t src)
{
    uint32_t dest;

    dest = float64_to_int32_round_to_zero(src, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tintrz_w_s(CPULoongArchState *env, uint32_t src)
{
    uint32_t dest;

    dest = float32_to_int32_round_to_zero(src, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tintrne_l_d(CPULoongArchState *env, uint64_t src)
{
    uint64_t dest;

    set_float_rounding_mode(float_round_nearest_even,
                            &env->active_fpu.fp_status);
    dest = float64_to_int64(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tintrne_l_s(CPULoongArchState *env, uint32_t src)
{
    uint64_t dest;

    set_float_rounding_mode(float_round_nearest_even,
                            &env->active_fpu.fp_status);
    dest = float32_to_int64(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tintrne_w_d(CPULoongArchState *env, uint64_t src)
{
    uint32_t dest;

    set_float_rounding_mode(float_round_nearest_even,
                            &env->active_fpu.fp_status);
    dest = float64_to_int32(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tintrne_w_s(CPULoongArchState *env, uint32_t src)
{
    uint32_t dest;

    set_float_rounding_mode(float_round_nearest_even,
                            &env->active_fpu.fp_status);
    dest = float32_to_int32(src, &env->active_fpu.fp_status);
    restore_rounding_mode(env);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tint_l_d(CPULoongArchState *env, uint64_t src)
{
    uint64_t dest;

    dest = float64_to_int64(src, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_tint_l_s(CPULoongArchState *env, uint32_t src)
{
    uint64_t dest;

    dest = float32_to_int64(src, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT64_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tint_w_s(CPULoongArchState *env, uint32_t src)
{
    uint32_t dest;

    dest = float32_to_int32(src, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_tint_w_d(CPULoongArchState *env, uint64_t src)
{
    uint32_t dest;

    dest = float64_to_int32(src, &env->active_fpu.fp_status);
    if (get_float_exception_flags(&env->active_fpu.fp_status)
        & (float_flag_invalid | float_flag_overflow)) {
        dest = FP_TO_INT32_OVERFLOW;
    }
    update_fcsr0(env, GETPC());
    return dest;
}

uint32_t helper_fp_rint_s(CPULoongArchState *env, uint32_t src)
{
    uint32_t dest;

    dest = float32_round_to_int(src, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return dest;
}

uint64_t helper_fp_rint_d(CPULoongArchState *env, uint64_t src)
{
    uint64_t dest;

    dest = float64_round_to_int(src, &env->active_fpu.fp_status);
    update_fcsr0(env, GETPC());
    return dest;
}
