/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * QEMU LoongArch CPU -- internal functions and types
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 */

#ifndef LOONGARCH_INTERNALS_H
#define LOONGARCH_INTERNALS_H

#define FCMP_LT   0b0001  /* fp0 < fp1 */
#define FCMP_EQ   0b0010  /* fp0 = fp1 */
#define FCMP_UN   0b0100  /* unordered */
#define FCMP_GT   0b1000  /* fp0 > fp1 */

#define TARGET_PHYS_MASK MAKE_64BIT_MASK(0, TARGET_PHYS_ADDR_SPACE_BITS)
#define TARGET_VIRT_MASK MAKE_64BIT_MASK(0, TARGET_VIRT_ADDR_SPACE_BITS)

/* Global bit used for lddir/ldpte */
#define LOONGARCH_PAGE_HUGE_SHIFT   6
/* Global bit for huge page */
#define LOONGARCH_HGLOBAL_SHIFT     12

void loongarch_translate_init(void);

void loongarch_cpu_dump_state(CPUState *cpu, FILE *f, int flags);

void QEMU_NORETURN do_raise_exception(CPULoongArchState *env,
                                      uint32_t exception,
                                      uintptr_t pc);

const char *loongarch_exception_name(int32_t exception);

void restore_fp_status(CPULoongArchState *env);

#ifndef CONFIG_USER_ONLY
extern const VMStateDescription vmstate_loongarch_cpu;

bool loongarch_cpu_tlb_fill(CPUState *cs, vaddr address, int size,
                            MMUAccessType access_type, int mmu_idx,
                            bool probe, uintptr_t retaddr);

void loongarch_mmu_init(CPULoongArchState *env);
hwaddr loongarch_cpu_get_phys_page_debug(CPUState *cpu, vaddr addr);
#endif
int loongarch_cpu_gdb_read_register(CPUState *cs, GByteArray *mem_buf, int n);
int loongarch_cpu_gdb_write_register(CPUState *cs, uint8_t *mem_buf, int n);
int loongarch_read_qxfer(CPUState *cs, const char *annex,
                         uint8_t *read_buf,
                         unsigned long offset, unsigned long len);
int loongarch_write_qxfer(CPUState *cs, const char *annex,
                          const uint8_t *write_buf,
                          unsigned long offset, unsigned long len);

void loongarch_cpu_register_gdb_regs_for_features(CPUState *cs);

#endif
