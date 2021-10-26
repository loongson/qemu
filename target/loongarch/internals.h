/*
 * QEMU LoongArch CPU -- internal functions and types
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef LOONGARCH_INTERNALS_H
#define LOONGARCH_INTERNALS_H

#define FCMP_LT   0x0001  /* fp0 < fp1 */
#define FCMP_EQ   0x0010  /* fp0 = fp1 */
#define FCMP_UN   0x0100  /* unordered */
#define FCMP_GT   0x1000  /* fp0 > fp1 */

void loongarch_translate_init(void);

void loongarch_cpu_dump_state(CPUState *cpu, FILE *f, int flags);

void QEMU_NORETURN do_raise_exception(CPULoongArchState *env,
                                      uint32_t exception,
                                      uintptr_t pc);

const char *loongarch_exception_name(int32_t exception);

void restore_fp_status(CPULoongArchState *env);

#ifndef CONFIG_USER_ONLY
extern const VMStateDescription vmstate_loongarch_cpu;
#endif

#endif
