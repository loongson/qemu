/*
 * QEMU LoongArch user cpu loop.
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "qemu.h"
#include "qemu-common.h"
#include "cpu_loop-common.h"
#include "elf.h"

/* Break codes */
enum {
    BRK_OVERFLOW = 6,
    BRK_DIVZERO = 7
};

static int do_break(CPULoongArchState *env, target_siginfo_t *info,
                    unsigned int code)
{
    int ret = -1;

    switch (code) {
    case BRK_OVERFLOW:
    case BRK_DIVZERO:
        info->si_signo = TARGET_SIGFPE;
        info->si_errno = 0;
        info->si_code = (code == BRK_OVERFLOW) ? FPE_INTOVF : FPE_INTDIV;
        queue_signal(env, info->si_signo, QEMU_SI_FAULT, &*info);
        ret = 0;
        break;
    default:
        info->si_signo = TARGET_SIGTRAP;
        info->si_errno = 0;
        queue_signal(env, info->si_signo, QEMU_SI_FAULT, &*info);
        ret = 0;
        break;
    }

    return ret;
}

void cpu_loop(CPULoongArchState *env)
{
    CPUState *cs = CPU(loongarch_env_get_cpu(env));
    target_siginfo_t info;
    int trapnr;
    abi_long ret;

    for (;;) {
        cpu_exec_start(cs);
        trapnr = cpu_exec(cs);
        cpu_exec_end(cs);
        process_queued_cpu_work(cs);

        switch (trapnr) {
        case EXCP_SYSCALL:
            env->active_tc.PC += 4;
            ret = do_syscall(env, env->active_tc.gpr[11],
                             env->active_tc.gpr[4], env->active_tc.gpr[5],
                             env->active_tc.gpr[6], env->active_tc.gpr[7],
                             env->active_tc.gpr[8], env->active_tc.gpr[9],
                             -1, -1);
            if (ret == -TARGET_ERESTARTSYS) {
                env->active_tc.PC -= 4;
                break;
            }
            if (ret == -TARGET_QEMU_ESIGRETURN) {
                /*
                 * Returning from a successful sigreturn syscall.
                 * Avoid clobbering register state.
                 */
                break;
            }
            env->active_tc.gpr[4] = ret;
            break;
        case EXCP_TLBL:
        case EXCP_TLBS:
        case EXCP_ADE:
            info.si_signo = TARGET_SIGSEGV;
            info.si_errno = 0;
            info.si_code = TARGET_SEGV_MAPERR;
            info._sifields._sigfault._addr = env->CSR_BADV;
            queue_signal(env, info.si_signo, QEMU_SI_FAULT, &info);
            break;
        case EXCP_FPDIS:
        case EXCP_INE:
            info.si_signo = TARGET_SIGILL;
            info.si_errno = 0;
            info.si_code = 0;
            queue_signal(env, info.si_signo, QEMU_SI_FAULT, &info);
            break;
        case EXCP_FPE:
            info.si_signo = TARGET_SIGFPE;
            info.si_errno = 0;
            info.si_code = TARGET_FPE_FLTUNK;
            if (GET_FP_CAUSE(env->active_fpu.fcsr0) & FP_INVALID) {
                info.si_code = TARGET_FPE_FLTINV;
            } else if (GET_FP_CAUSE(env->active_fpu.fcsr0) & FP_DIV0) {
                info.si_code = TARGET_FPE_FLTDIV;
            } else if (GET_FP_CAUSE(env->active_fpu.fcsr0) & FP_OVERFLOW) {
                info.si_code = TARGET_FPE_FLTOVF;
            } else if (GET_FP_CAUSE(env->active_fpu.fcsr0) & FP_UNDERFLOW) {
                info.si_code = TARGET_FPE_FLTUND;
            } else if (GET_FP_CAUSE(env->active_fpu.fcsr0) & FP_INEXACT) {
                info.si_code = TARGET_FPE_FLTRES;
            }
            queue_signal(env, info.si_signo, QEMU_SI_FAULT, &info);
            break;
        case EXCP_BREAK:
            {
                abi_ulong trap_instr;
                unsigned int code;

                ret = get_user_u32(trap_instr, env->active_tc.PC);
                if (ret != 0) {
                    abort();
                    goto error;
                }

                code = trap_instr & 0x7fff;

                if (do_break(env, &info, code) != 0) {
                    abort();
                    goto error;
                }
            }
            break;
        case EXCP_TRAP:
            {
                abi_ulong trap_instr;
                unsigned int code = 0;

                ret = get_user_u32(trap_instr, env->active_tc.PC);

                if (ret != 0) {
                    abort();
                    goto error;
                }

                /* The immediate versions don't provide a code. */
                if (!(trap_instr & 0xFC000000)) {
                    code = ((trap_instr >> 6) & ((1 << 10) - 1));
                }

                if (do_break(env, &info, code) != 0) {
                    abort();
                    goto error;
                }
            }
            break;
        case EXCP_ATOMIC:
            cpu_exec_step_atomic(cs);
            break;
        default:
error:
            EXCP_DUMP(env, "qemu: unhandled CPU exception 0x%x - aborting\n",
                      trapnr);
            abort();
        }
        process_pending_signals(env);
    }
}

void target_cpu_copy_regs(CPUArchState *env, struct target_pt_regs *regs)
{
    int i;

    for (i = 0; i < 32; i++) {
        env->active_tc.gpr[i] = regs->regs[i];
    }
    env->active_tc.PC = regs->csr_era & ~(target_ulong)1;

}
