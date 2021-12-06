/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * LoongArch emulation of Linux signals
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 */

#include "qemu/osdep.h"
#include "qemu.h"
#include "signal-common.h"
#include "user-internals.h"
#include "linux-user/trace.h"

#define FPU_REG_WIDTH 256
union fpureg {
    uint32_t val32[FPU_REG_WIDTH / 32];
    uint64_t val64[FPU_REG_WIDTH / 64];
};

struct target_sigcontext {
    uint64_t sc_pc;
    uint64_t sc_regs[32];
    uint32_t sc_flags;
    uint32_t sc_fcsr;
    uint32_t sc_vcsr;
    uint64_t sc_fcc;
    uint64_t sc_scr[4];
    union fpureg sc_fpregs[32] __attribute__((aligned(32)));
    uint8_t sc_reserved[4096] __attribute__((aligned(16)));
};

struct target_ucontext {
    target_ulong tuc_flags;
    struct target_ucontext *tuc_link;
    target_stack_t tuc_stack;
    target_sigset_t tuc_sigmask;
    uint8_t __unused[1024 / 8 - sizeof(target_sigset_t)];
    struct target_sigcontext tuc_mcontext;
};

struct target_rt_sigframe {
    struct target_siginfo rs_info;
    struct target_ucontext rs_uc;
};

static uint64_t read_all_fcc(CPULoongArchState *env)
{
    uint64_t ret = 0;

    for (int i = 0; i < 8; ++i) {
        ret |= (uint64_t)env->cf[i] << (i * 8);
    }

    return ret;
}

static void write_all_fcc(CPULoongArchState *env, uint64_t val)
{
    for (int i = 0; i < 8; ++i) {
        env->cf[i] = (val >> (i * 8)) & 1;
    }
}

static inline void setup_sigcontext(CPULoongArchState *env,
                                    struct target_sigcontext *sc)
{
    int i;

    __put_user(env->pc, &sc->sc_pc);
    __put_user(0, &sc->sc_regs[0]);
    __put_user(env->fcsr0, &sc->sc_fcsr);
    __put_user(0, &sc->sc_vcsr);
    sc->sc_fcc = read_all_fcc(env);

    for (i = 0; i < 4; ++i) {
        __put_user(0, &sc->sc_scr[i]);
    }

    for (i = 1; i < 32; ++i) {
        __put_user(env->gpr[i], &sc->sc_regs[i]);
    }

    for (i = 0; i < 32; ++i) {
        __put_user(env->fpr[i], &sc->sc_fpregs[i].val64[0]);
    }
}

static inline void
restore_sigcontext(CPULoongArchState *env, struct target_sigcontext *sc)
{
    int i;

    __get_user(env->pc, &sc->sc_pc);
    __get_user(env->fcsr0, &sc->sc_fcsr);
    write_all_fcc(env, sc->sc_fcc);

    for (i = 1; i < 32; ++i) {
        __get_user(env->gpr[i], &sc->sc_regs[i]);
    }

    for (i = 0; i < 32; ++i) {
        __get_user(env->fpr[i], &sc->sc_fpregs[i].val64[0]);
    }
}

/*
 * Determine which stack to use..
 */
static inline abi_ulong
get_sigframe(struct target_sigaction *ka, CPULoongArchState *env,
             size_t frame_size)
{
    unsigned long sp;

    sp = target_sigsp(get_sp_from_cpustate(env) - 32, ka);

    return (sp - frame_size) & ~15;
}

void setup_rt_frame(int sig, struct target_sigaction *ka,
                    target_siginfo_t *info,
                    target_sigset_t *set, CPULoongArchState *env)
{
    struct target_rt_sigframe *frame;
    abi_ulong frame_addr;
    int i;

    frame_addr = get_sigframe(ka, env, sizeof(*frame));
    trace_user_setup_rt_frame(env, frame_addr);
    if (!lock_user_struct(VERIFY_WRITE, frame, frame_addr, 0)) {
        goto give_sigsegv;
    }

    tswap_siginfo(&frame->rs_info, info);

    __put_user(0, &frame->rs_uc.tuc_flags);
    __put_user(0, &frame->rs_uc.tuc_link);
    target_save_altstack(&frame->rs_uc.tuc_stack, env);

    setup_sigcontext(env, &frame->rs_uc.tuc_mcontext);

    for (i = 0; i < TARGET_NSIG_WORDS; i++) {
        __put_user(set->sig[i], &frame->rs_uc.tuc_sigmask.sig[i]);
    }

    env->gpr[4] = sig;
    env->gpr[5] = frame_addr + offsetof(struct target_rt_sigframe, rs_info);
    env->gpr[6] = frame_addr + offsetof(struct target_rt_sigframe, rs_uc);
    env->gpr[3] = frame_addr;
    env->gpr[1] = default_rt_sigreturn;

    env->pc = ka->_sa_handler;
    unlock_user_struct(frame, frame_addr, 1);
    return;

give_sigsegv:
    unlock_user_struct(frame, frame_addr, 1);
    force_sigsegv(sig);
}

long do_rt_sigreturn(CPULoongArchState *env)
{
    struct target_rt_sigframe *frame;
    abi_ulong frame_addr;
    sigset_t blocked;

    frame_addr = env->gpr[3];
    trace_user_do_rt_sigreturn(env, frame_addr);
    if (!lock_user_struct(VERIFY_READ, frame, frame_addr, 1)) {
        goto badframe;
    }

    target_to_host_sigset(&blocked, &frame->rs_uc.tuc_sigmask);
    set_sigmask(&blocked);

    restore_sigcontext(env, &frame->rs_uc.tuc_mcontext);
    target_restore_altstack(&frame->rs_uc.tuc_stack, env);

    unlock_user_struct(frame, frame_addr, 0);
    return -TARGET_QEMU_ESIGRETURN;

badframe:
    unlock_user_struct(frame, frame_addr, 0);
    force_sig(TARGET_SIGSEGV);
    return -TARGET_QEMU_ESIGRETURN;
}

void setup_sigtramp(abi_ulong sigtramp_page)
{
    uint32_t *tramp = lock_user(VERIFY_WRITE, sigtramp_page, 8, 0);
    assert(tramp != NULL);

    __put_user(0x03822c0b, tramp + 0);  /* ori     a7, zero, 0x8b */
    __put_user(0x002b0000, tramp + 1);  /* syscall 0 */

    default_rt_sigreturn = sigtramp_page;
    unlock_user(tramp, sigtramp_page, 8);
}
