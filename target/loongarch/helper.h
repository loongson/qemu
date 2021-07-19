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
