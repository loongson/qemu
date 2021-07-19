/*
 * QEMU LoongArch CSR
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef _CPU_CSR_H_
#define _CPU_CSR_H_

/*
 * basic CSR register copy
 * copy from kernel arch/loongarch/include/asm/loongarchregs.h
 */

#define LOONGARCH_CSR_CRMD           0x0 /* Current mode info */
#define  CSR_CRMD_DACM_SHIFT         7
#define  CSR_CRMD_DACM_WIDTH         2
#define  CSR_CRMD_DACM               (0x3UL << CSR_CRMD_DACM_SHIFT)
#define  CSR_CRMD_DACF_SHIFT         5
#define  CSR_CRMD_DACF_WIDTH         2
#define  CSR_CRMD_DACF               (0x3UL << CSR_CRMD_DACF_SHIFT)
#define  CSR_CRMD_PG_SHIFT           4
#define  CSR_CRMD_PG                 (0x1UL << CSR_CRMD_PG_SHIFT)
#define  CSR_CRMD_DA_SHIFT           3
#define  CSR_CRMD_DA                 (0x1UL << CSR_CRMD_DA_SHIFT)
#define  CSR_CRMD_IE_SHIFT           2
#define  CSR_CRMD_IE                 (0x1UL << CSR_CRMD_IE_SHIFT)
#define  CSR_CRMD_PLV_SHIFT          0
#define  CSR_CRMD_PLV_WIDTH          2
#define  CSR_CRMD_PLV                (0x3UL << CSR_CRMD_PLV_SHIFT)

#define PLV_USER                     3
#define PLV_KERN                     0
#define PLV_MASK                     0x3

#define LOONGARCH_CSR_PRMD           0x1 /* Prev-exception mode info */
#define  CSR_PRMD_PIE_SHIFT          2
#define  CSR_PRMD_PIE                (0x1UL << CSR_PRMD_PIE_SHIFT)
#define  CSR_PRMD_PPLV_SHIFT         0
#define  CSR_PRMD_PPLV_WIDTH         2
#define  CSR_PRMD_PPLV               (0x3UL << CSR_PRMD_PPLV_SHIFT)

#define LOONGARCH_CSR_EUEN           0x2 /* Extended unit enable */
#define  CSR_EUEN_LBTEN_SHIFT        3
#define  CSR_EUEN_LBTEN              (0x1UL << CSR_EUEN_LBTEN_SHIFT)
#define  CSR_EUEN_LASXEN_SHIFT       2
#define  CSR_EUEN_LASXEN             (0x1UL << CSR_EUEN_LASXEN_SHIFT)
#define  CSR_EUEN_LSXEN_SHIFT        1
#define  CSR_EUEN_LSXEN              (0x1UL << CSR_EUEN_LSXEN_SHIFT)
#define  CSR_EUEN_FPEN_SHIFT         0
#define  CSR_EUEN_FPEN               (0x1UL << CSR_EUEN_FPEN_SHIFT)

#define LOONGARCH_CSR_MISC           0x3 /* Misc config */

#define LOONGARCH_CSR_ECFG           0x4 /* Exception config */
#define  CSR_ECFG_VS_SHIFT           16
#define  CSR_ECFG_VS_WIDTH           3
#define  CSR_ECFG_VS                 (0x7UL << CSR_ECFG_VS_SHIFT)
#define  CSR_ECFG_IM_SHIFT           0
#define  CSR_ECFG_IM_WIDTH           13
#define  CSR_ECFG_IM                 (0x1fffUL << CSR_ECFG_IM_SHIFT)

#define  CSR_ECFG_IPMASK             0x00001fff

#define LOONGARCH_CSR_ESTAT          0x5 /* Exception status */
#define  CSR_ESTAT_ESUBCODE_SHIFT    22
#define  CSR_ESTAT_ESUBCODE_WIDTH    9
#define  CSR_ESTAT_ESUBCODE          (0x1ffULL << CSR_ESTAT_ESUBCODE_SHIFT)
#define  CSR_ESTAT_EXC_SH            16
#define  CSR_ESTAT_EXC_WIDTH         5
#define  CSR_ESTAT_EXC               (0x1fULL << CSR_ESTAT_EXC_SH)
#define  CSR_ESTAT_IS_SHIFT          0
#define  CSR_ESTAT_IS_WIDTH          15
#define  CSR_ESTAT_IS                (0x7fffULL << CSR_ESTAT_IS_SHIFT)

#define  CSR_ESTAT_IPMASK            0x00001fff

/*
 * ExStatus.ExcCode
 */
#define  EXCCODE_INT_START           64
#define  EXCCODE_RSV                 0    /* Reserved */
#define  EXCCODE_TLBL                1    /* TLB miss on a load */
#define  EXCCODE_TLBS                2    /* TLB miss on a store */
#define  EXCCODE_TLBI                3    /* TLB miss on a ifetch */
#define  EXCCODE_TLBM                4    /* TLB modified fault */
#define  EXCCODE_TLBRI               5    /* TLB Read-Inhibit exception */
#define  EXCCODE_TLBXI               6    /* TLB Execution-Inhibit exception */
#define  EXCCODE_TLBPE               7    /* TLB Privilege Error */
#define  EXCCODE_ADE                 8    /* Address Error */
#define  EXCCODE_ALE                 9    /* Unalign Access */
#define  EXCCODE_OOB                 10   /* Out of bounds */
#define  EXCCODE_SYS                 11   /* System call */
#define  EXCCODE_BP                  12   /* Breakpoint */
#define  EXCCODE_INE                 13   /* Inst. Not Exist */
#define  EXCCODE_IPE                 14   /* Inst. Privileged Error */
#define  EXCCODE_FPDIS               15   /* FPU Disabled */
#define  EXCCODE_LSXDIS              16   /* LSX Disabled */
#define  EXCCODE_LASXDIS             17   /* LASX Disabled */
#define  EXCCODE_FPE                 18   /* Floating Point Exception */
#define  EXCCODE_WATCH               19   /* Watch address reference */
#define  EXCCODE_BTDIS               20   /* Binary Trans. Disabled */
#define  EXCCODE_BTE                 21   /* Binary Trans. Exception */
#define  EXCCODE_PSI                 22   /* Guest Privileged Error */
#define  EXCCODE_HYP                 23   /* Hypercall */
#define  EXCCODE_GCM                 24   /* Guest CSR modified */
#define  EXCCODE_SE                  25   /* Security*/

#define LOONGARCH_CSR_ERA            0x6  /* Error PC */

#define LOONGARCH_CSR_BADV           0x7  /* Bad virtual address */

#define LOONGARCH_CSR_BADI           0x8  /* Bad instruction */

#define LOONGARCH_CSR_EEPN           0xc  /* Exception enter base address */

/* TLB related CSR register */
#define LOONGARCH_CSR_TLBIDX         0x10 /* TLB Index, EHINV, PageSize, NP*/
#define  CSR_TLBIDX_EHINV_SHIFT      31
#define  CSR_TLBIDX_EHINV            (0x1ULL << CSR_TLBIDX_EHINV_SHIFT)
#define  CSR_TLBIDX_PS_SHIFT         24
#define  CSR_TLBIDX_PS_WIDTH         6
#define  CSR_TLBIDX_PS               (0x3fULL << CSR_TLBIDX_PS_SHIFT)
#define  CSR_TLBIDX_IDX_SHIFT        0
#define  CSR_TLBIDX_IDX_WIDTH        12
#define  CSR_TLBIDX_IDX              (0xfffULL << CSR_TLBIDX_IDX_SHIFT)
#define  CSR_TLBIDX_SIZEM            0x3f000000
#define  CSR_TLBIDX_SIZE             CSR_TLBIDX_PS_SHIFT
#define  CSR_TLBIDX_IDXM             0xfff

#define LOONGARCH_CSR_TLBEHI         0x11 /* TLB EntryHi without ASID */

#define LOONGARCH_CSR_TLBELO0        0x12 /* TLB EntryLo0 */
#define  CSR_TLBLO0_RPLV_SHIFT       63
#define  CSR_TLBLO0_RPLV             (0x1ULL << CSR_TLBLO0_RPLV_SHIFT)
#define  CSR_TLBLO0_XI_SHIFT         62
#define  CSR_TLBLO0_XI               (0x1ULL << CSR_TLBLO0_XI_SHIFT)
#define  CSR_TLBLO0_RI_SHIFT         61
#define  CSR_TLBLO0_RI               (0x1ULL << CSR_TLBLO0_RI_SHIFT)
#define  CSR_TLBLO0_PPN_SHIFT        12
#define  CSR_TLBLO0_PPN_WIDTH        36 /* ignore lower 12bits */
#define  CSR_TLBLO0_PPN              (0xfffffffffULL << CSR_TLBLO0_PPN_SHIFT)
#define  CSR_TLBLO0_GLOBAL_SHIFT     6
#define  CSR_TLBLO0_GLOBAL           (0x1ULL << CSR_TLBLO0_GLOBAL_SHIFT)
#define  CSR_TLBLO0_CCA_SHIFT        4
#define  CSR_TLBLO0_CCA_WIDTH        2
#define  CSR_TLBLO0_CCA              (0x3ULL << CSR_TLBLO0_CCA_SHIFT)
#define  CSR_TLBLO0_PLV_SHIFT        2
#define  CSR_TLBLO0_PLV_WIDTH        2
#define  CSR_TLBLO0_PLV              (0x3ULL << CSR_TLBLO0_PLV_SHIFT)
#define  CSR_TLBLO0_WE_SHIFT         1
#define  CSR_TLBLO0_WE               (0x1ULL << CSR_TLBLO0_WE_SHIFT)
#define  CSR_TLBLO0_V_SHIFT          0
#define  CSR_TLBLO0_V                (0x1ULL << CSR_TLBLO0_V_SHIFT)

#define LOONGARCH_CSR_TLBELO1        0x13 /* TLB EntryLo1 */
#define  CSR_TLBLO1_RPLV_SHIFT       63
#define  CSR_TLBLO1_RPLV             (0x1ULL << CSR_TLBLO1_RPLV_SHIFT)
#define  CSR_TLBLO1_XI_SHIFT         62
#define  CSR_TLBLO1_XI               (0x1ULL << CSR_TLBLO1_XI_SHIFT)
#define  CSR_TLBLO1_RI_SHIFT         61
#define  CSR_TLBLO1_RI               (0x1ULL << CSR_TLBLO1_RI_SHIFT)
#define  CSR_TLBLO1_PPN_SHIFT        12
#define  CSR_TLBLO1_PPN_WIDTH        36
#define  CSR_TLBLO1_PPN              (0xfffffffffULL << CSR_TLBLO1_PPN_SHIFT)
#define  CSR_TLBLO1_GLOBAL_SHIFT     6
#define  CSR_TLBLO1_GLOBAL           (0x1ULL << CSR_TLBLO1_GLOBAL_SHIFT)
#define  CSR_TLBLO1_CCA_SHIFT        4
#define  CSR_TLBLO1_CCA_WIDTH        2
#define  CSR_TLBLO1_CCA              (0x3ULL << CSR_TLBLO1_CCA_SHIFT)
#define  CSR_TLBLO1_PLV_SHIFT        2
#define  CSR_TLBLO1_PLV_WIDTH        2
#define  CSR_TLBLO1_PLV              (0x3ULL << CSR_TLBLO1_PLV_SHIFT)
#define  CSR_TLBLO1_WE_SHIFT         1
#define  CSR_TLBLO1_WE               (0x1ULL << CSR_TLBLO1_WE_SHIFT)
#define  CSR_TLBLO1_V_SHIFT          0
#define  CSR_TLBLO1_V                (0x1ULL << CSR_TLBLO1_V_SHIFT)

#define LOONGARCH_CSR_ASID           0x18 /* 64 ASID */
#define  CSR_ASID_BIT_SHIFT          16 /* ASIDBits */
#define  CSR_ASID_BIT_WIDTH          8
#define  CSR_ASID_BIT                (0xffULL << CSR_ASID_BIT_SHIFT)
#define  CSR_ASID_ASID_SHIFT         0
#define  CSR_ASID_ASID_WIDTH         10
#define  CSR_ASID_ASID               (0x3ffULL << CSR_ASID_ASID_SHIFT)

/* Page table base address when badv[47] = 0 */
#define LOONGARCH_CSR_PGDL           0x19
/* Page table base address when badv[47] = 1 */
#define LOONGARCH_CSR_PGDH           0x1a

#define LOONGARCH_CSR_PGD            0x1b /* Page table base */

#define LOONGARCH_CSR_PWCTL0         0x1c /* PWCtl0 */
#define  CSR_PWCTL0_PTEW_SHIFT       30
#define  CSR_PWCTL0_PTEW_WIDTH       2
#define  CSR_PWCTL0_PTEW             (0x3ULL << CSR_PWCTL0_PTEW_SHIFT)
#define  CSR_PWCTL0_DIR1WIDTH_SHIFT  25
#define  CSR_PWCTL0_DIR1WIDTH_WIDTH  5
#define  CSR_PWCTL0_DIR1WIDTH        (0x1fULL << CSR_PWCTL0_DIR1WIDTH_SHIFT)
#define  CSR_PWCTL0_DIR1BASE_SHIFT   20
#define  CSR_PWCTL0_DIR1BASE_WIDTH   5
#define  CSR_PWCTL0_DIR1BASE         (0x1fULL << CSR_PWCTL0_DIR1BASE_SHIFT)
#define  CSR_PWCTL0_DIR0WIDTH_SHIFT  15
#define  CSR_PWCTL0_DIR0WIDTH_WIDTH  5
#define  CSR_PWCTL0_DIR0WIDTH        (0x1fULL << CSR_PWCTL0_DIR0WIDTH_SHIFT)
#define  CSR_PWCTL0_DIR0BASE_SHIFT   10
#define  CSR_PWCTL0_DIR0BASE_WIDTH   5
#define  CSR_PWCTL0_DIR0BASE         (0x1fULL << CSR_PWCTL0_DIR0BASE_SHIFT)
#define  CSR_PWCTL0_PTWIDTH_SHIFT    5
#define  CSR_PWCTL0_PTWIDTH_WIDTH    5
#define  CSR_PWCTL0_PTWIDTH          (0x1fULL << CSR_PWCTL0_PTWIDTH_SHIFT)
#define  CSR_PWCTL0_PTBASE_SHIFT     0
#define  CSR_PWCTL0_PTBASE_WIDTH     5
#define  CSR_PWCTL0_PTBASE           (0x1fULL << CSR_PWCTL0_PTBASE_SHIFT)

#define LOONGARCH_CSR_PWCTL1         0x1d /* PWCtl1 */
#define  CSR_PWCTL1_DIR3WIDTH_SHIFT  18
#define  CSR_PWCTL1_DIR3WIDTH_WIDTH  5
#define  CSR_PWCTL1_DIR3WIDTH        (0x1fULL << CSR_PWCTL1_DIR3WIDTH_SHIFT)
#define  CSR_PWCTL1_DIR3BASE_SHIFT   12
#define  CSR_PWCTL1_DIR3BASE_WIDTH   5
#define  CSR_PWCTL1_DIR3BASE         (0x1fULL << CSR_PWCTL0_DIR3BASE_SHIFT)
#define  CSR_PWCTL1_DIR2WIDTH_SHIFT  6
#define  CSR_PWCTL1_DIR2WIDTH_WIDTH  5
#define  CSR_PWCTL1_DIR2WIDTH        (0x1fULL << CSR_PWCTL1_DIR2WIDTH_SHIFT)
#define  CSR_PWCTL1_DIR2BASE_SHIFT   0
#define  CSR_PWCTL1_DIR2BASE_WIDTH   5
#define  CSR_PWCTL1_DIR2BASE         (0x1fULL << CSR_PWCTL0_DIR2BASE_SHIFT)

#define LOONGARCH_CSR_STLBPGSIZE     0x1e
#define  CSR_STLBPGSIZE_PS_WIDTH     6
#define  CSR_STLBPGSIZE_PS           (0x3f)

#define LOONGARCH_CSR_RVACFG         0x1f
#define  CSR_RVACFG_RDVA_WIDTH       4
#define  CSR_RVACFG_RDVA             (0xf)

/* Config CSR registers */
#define LOONGARCH_CSR_CPUID          0x20 /* CPU core number */
#define  CSR_CPUID_CID_WIDTH         9
#define  CSR_CPUID_CID               (0x1ff)

#define LOONGARCH_CSR_PRCFG1         0x21 /* Config1 */
#define  CSR_CONF1_VSMAX_SHIFT       12
#define  CSR_CONF1_VSMAX_WIDTH       3
#define  CSR_CONF1_VSMAX             (7ULL << CSR_CONF1_VSMAX_SHIFT)
#define  CSR_CONF1_TMRBITS_SHIFT     4
#define  CSR_CONF1_TMRBITS_WIDTH     8
#define  CSR_CONF1_TMRBITS           (0xffULL << CSR_CONF1_TMRBITS_SHIFT)
#define  CSR_CONF1_KSNUM_SHIFT       0
#define  CSR_CONF1_KSNUM_WIDTH       4
#define  CSR_CONF1_KSNUM             (0x8)

#define LOONGARCH_CSR_PRCFG2         0x22 /* Config2 */
#define  CSR_CONF2_PGMASK_SUPP       0x3ffff000

#define LOONGARCH_CSR_PRCFG3         0x23 /* Config3 */
#define  CSR_CONF3_STLBIDX_SHIFT     20
#define  CSR_CONF3_STLBIDX_WIDTH     6
#define  CSR_CONF3_STLBIDX           (0x3fULL << CSR_CONF3_STLBIDX_SHIFT)
#define  CSR_CONF3_STLBWAYS_SHIFT    12
#define  CSR_CONF3_STLBWAYS_WIDTH    8
#define  CSR_CONF3_STLBWAYS          (0xffULL << CSR_CONF3_STLBWAYS_SHIFT)
#define  CSR_CONF3_MTLBSIZE_SHIFT    4
#define  CSR_CONF3_MTLBSIZE_WIDTH    8
#define  CSR_CONF3_MTLBSIZE          (0xffULL << CSR_CONF3_MTLBSIZE_SHIFT)
#define  CSR_CONF3_TLBORG_SHIFT      0
#define  CSR_CONF3_TLBORG_WIDTH      4
#define  CSR_CONF3_TLBORG            (0xfULL << CSR_CONF3_TLBORG_SHIFT)

/* Kscratch registers */
#define LOONGARCH_CSR_KS0            0x30
#define LOONGARCH_CSR_KS1            0x31
#define LOONGARCH_CSR_KS2            0x32
#define LOONGARCH_CSR_KS3            0x33
#define LOONGARCH_CSR_KS4            0x34
#define LOONGARCH_CSR_KS5            0x35
#define LOONGARCH_CSR_KS6            0x36
#define LOONGARCH_CSR_KS7            0x37
#define LOONGARCH_CSR_KS8            0x38

/* Timer registers */
#define LOONGARCH_CSR_TMID           0x40 /* Timer ID */

#define LOONGARCH_CSR_TCFG           0x41 /* Timer config */
#define  CSR_TCFG_VAL_SHIFT          2
#define  CSR_TCFG_VAL_WIDTH          48
#define  CSR_TCFG_VAL                (0x3fffffffffffULL << CSR_TCFG_VAL_SHIFT)
#define  CSR_TCFG_PERIOD_SHIFT       1
#define  CSR_TCFG_PERIOD             (0x1ULL << CSR_TCFG_PERIOD_SHIFT)
#define  CSR_TCFG_EN                 (0x1)

#define LOONGARCH_CSR_TVAL           0x42 /* Timer value */

#define LOONGARCH_CSR_CNTC           0x43 /* Timer offset */

#define LOONGARCH_CSR_TINTCLR        0x44 /* Timer interrupt clear */
#define  CSR_TINTCLR_TI_SHIFT        0
#define  CSR_TINTCLR_TI              (1 << CSR_TINTCLR_TI_SHIFT)

/* LLBCTL register */
#define LOONGARCH_CSR_LLBIT          0x60 /* LLBit control */
#define  CSR_LLBIT_ROLLB_SHIFT       0
#define  CSR_LLBIT_ROLLB             (1ULL << CSR_LLBIT_ROLLB_SHIFT)
#define  CSR_LLBIT_WCLLB_SHIFT       1
#define  CSR_LLBIT_WCLLB             (1ULL << CSR_LLBIT_WCLLB_SHIFT)
#define  CSR_LLBIT_KLO_SHIFT         2
#define  CSR_LLBIT_KLO               (1ULL << CSR_LLBIT_KLO_SHIFT)

/* Implement dependent */
#define LOONGARCH_CSR_IMPCTL1        0x80 /* Loongarch config1 */
#define  CSR_MISPEC_SHIFT            20
#define  CSR_MISPEC_WIDTH            8
#define  CSR_MISPEC                  (0xffULL << CSR_MISPEC_SHIFT)
#define  CSR_SSEN_SHIFT              18
#define  CSR_SSEN                    (1ULL << CSR_SSEN_SHIFT)
#define  CSR_SCRAND_SHIFT            17
#define  CSR_SCRAND                  (1ULL << CSR_SCRAND_SHIFT)
#define  CSR_LLEXCL_SHIFT            16
#define  CSR_LLEXCL                  (1ULL << CSR_LLEXCL_SHIFT)
#define  CSR_DISVC_SHIFT             15
#define  CSR_DISVC                   (1ULL << CSR_DISVC_SHIFT)
#define  CSR_VCLRU_SHIFT             14
#define  CSR_VCLRU                   (1ULL << CSR_VCLRU_SHIFT)
#define  CSR_DCLRU_SHIFT             13
#define  CSR_DCLRU                   (1ULL << CSR_DCLRU_SHIFT)
#define  CSR_FASTLDQ_SHIFT           12
#define  CSR_FASTLDQ                 (1ULL << CSR_FASTLDQ_SHIFT)
#define  CSR_USERCAC_SHIFT           11
#define  CSR_USERCAC                 (1ULL << CSR_USERCAC_SHIFT)
#define  CSR_ANTI_MISPEC_SHIFT       10
#define  CSR_ANTI_MISPEC             (1ULL << CSR_ANTI_MISPEC_SHIFT)
#define  CSR_ANTI_FLUSHSFB_SHIFT     9
#define  CSR_ANTI_FLUSHSFB           (1ULL << CSR_ANTI_FLUSHSFB_SHIFT)
#define  CSR_STFILL_SHIFT            8
#define  CSR_STFILL                  (1ULL << CSR_STFILL_SHIFT)
#define  CSR_LIFEP_SHIFT             7
#define  CSR_LIFEP                   (1ULL << CSR_LIFEP_SHIFT)
#define  CSR_LLSYNC_SHIFT            6
#define  CSR_LLSYNC                  (1ULL << CSR_LLSYNC_SHIFT)
#define  CSR_BRBTDIS_SHIFT           5
#define  CSR_BRBTDIS                 (1ULL << CSR_BRBTDIS_SHIFT)
#define  CSR_RASDIS_SHIFT            4
#define  CSR_RASDIS                  (1ULL << CSR_RASDIS_SHIFT)
#define  CSR_STPRE_SHIFT             2
#define  CSR_STPRE_WIDTH             2
#define  CSR_STPRE                   (3ULL << CSR_STPRE_SHIFT)
#define  CSR_INSTPRE_SHIFT           1
#define  CSR_INSTPRE                 (1ULL << CSR_INSTPRE_SHIFT)
#define  CSR_DATAPRE_SHIFT           0
#define  CSR_DATAPRE                 (1ULL << CSR_DATAPRE_SHIFT)

#define LOONGARCH_CSR_IMPCTL2        0x81 /* loongarch config2 */
#define  CSR_IMPCTL2_MTLB_SHIFT      0
#define  CSR_IMPCTL2_MTLB            (1ULL << CSR_IMPCTL2_MTLB_SHIFT)
#define  CSR_IMPCTL2_STLB_SHIFT      1
#define  CSR_IMPCTL2_STLB            (1ULL << CSR_IMPCTL2_STLB_SHIFT)
#define  CSR_IMPCTL2_DTLB_SHIFT      2
#define  CSR_IMPCTL2_DTLB            (1ULL << CSR_IMPCTL2_DTLB_SHIFT)
#define  CSR_IMPCTL2_ITLB_SHIFT      3
#define  CSR_IMPCTL2_ITLB            (1ULL << CSR_IMPCTL2_ITLB_SHIFT)
#define  CSR_IMPCTL2_BTAC_SHIFT      4
#define  CSR_IMPCTL2_BTAC            (1ULL << CSR_IMPCTL2_BTAC_SHIFT)

#define LOONGARCH_CSR_GNMI           0x82

/* TLB refill registers */
#define LOONGARCH_CSR_TLBRENT        0x88 /* TLB refill exception address */
#define LOONGARCH_CSR_TLBRBADV       0x89 /* TLB refill badvaddr */
#define LOONGARCH_CSR_TLBRERA        0x8a /* TLB refill ERA */
#define LOONGARCH_CSR_TLBRSAVE       0x8b /* KScratch for TLB refill */
#define LOONGARCH_CSR_TLBRELO0       0x8c /* TLB refill entrylo0 */
#define LOONGARCH_CSR_TLBRELO1       0x8d /* TLB refill entrylo1 */
#define LOONGARCH_CSR_TLBREHI        0x8e /* TLB refill entryhi */
#define LOONGARCH_CSR_TLBRPRMD       0x8f /* TLB refill mode info */

/* Machine error registers */
#define LOONGARCH_CSR_ERRCTL         0x90 /* ERRCTL */
#define LOONGARCH_CSR_ERRINFO        0x91 /* Error info1 */
#define LOONGARCH_CSR_ERRINFO1       0x92 /* Error info2 */
#define LOONGARCH_CSR_ERRENT         0x93 /* Error exception base address */
#define LOONGARCH_CSR_ERRERA         0x94 /* Error exception PC */
#define LOONGARCH_CSR_ERRSAVE        0x95 /* KScratch machine error exception */

#define LOONGARCH_CSR_CTAG           0x98 /* TagLo + TagHi */

/* Shadow MCSR : 0xc0 ~ 0xff */
#define LOONGARCH_CSR_MCSR0          0xc0 /* CPUCFG0 and CPUCFG1 */
#define  MCSR0_INT_IMPL_SHIFT        58
#define  MCSR0_INT_IMPL              0
#define  MCSR0_IOCSR_BRD_SHIFT       57
#define  MCSR0_IOCSR_BRD             (1ULL << MCSR0_IOCSR_BRD_SHIFT)
#define  MCSR0_HUGEPG_SHIFT          56
#define  MCSR0_HUGEPG                (1ULL << MCSR0_HUGEPG_SHIFT)
#define  MCSR0_RPLVTLB_SHIFT         55
#define  MCSR0_RPLVTLB               (1ULL << MCSR0_RPLVTLB_SHIFT)
#define  MCSR0_EXEPROT_SHIFT         54
#define  MCSR0_EXEPROT               (1ULL << MCSR0_EXEPROT_SHIFT)
#define  MCSR0_RI_SHIFT              53
#define  MCSR0_RI                    (1ULL << MCSR0_RI_SHIFT)
#define  MCSR0_UAL_SHIFT             52
#define  MCSR0_UAL                   (1ULL << MCSR0_UAL_SHIFT)
#define  MCSR0_VABIT_SHIFT           44
#define  MCSR0_VABIT_WIDTH           8
#define  MCSR0_VABIT                 (0xffULL << MCSR0_VABIT_SHIFT)
#define  VABIT_DEFAULT               0x2f
#define  MCSR0_PABIT_SHIFT           36
#define  MCSR0_PABIT_WIDTH           8
#define  MCSR0_PABIT                 (0xffULL << MCSR0_PABIT_SHIFT)
#define  PABIT_DEFAULT               0x2f
#define  MCSR0_IOCSR_SHIFT           35
#define  MCSR0_IOCSR                 (1ULL << MCSR0_IOCSR_SHIFT)
#define  MCSR0_PAGING_SHIFT          34
#define  MCSR0_PAGING                (1ULL << MCSR0_PAGING_SHIFT)
#define  MCSR0_GR64_SHIFT            33
#define  MCSR0_GR64                  (1ULL << MCSR0_GR64_SHIFT)
#define  GR64_DEFAULT                1
#define  MCSR0_GR32_SHIFT            32
#define  MCSR0_GR32                  (1ULL << MCSR0_GR32_SHIFT)
#define  GR32_DEFAULT                0
#define  MCSR0_PRID_WIDTH            32
#define  MCSR0_PRID                  0x14C010

#define LOONGARCH_CSR_MCSR1          0xc1 /* CPUCFG2 and CPUCFG3 */
#define  MCSR1_HPFOLD_SHIFT          43
#define  MCSR1_HPFOLD                (1ULL << MCSR1_HPFOLD_SHIFT)
#define  MCSR1_SPW_LVL_SHIFT         40
#define  MCSR1_SPW_LVL_WIDTH         3
#define  MCSR1_SPW_LVL               (7ULL << MCSR1_SPW_LVL_SHIFT)
#define  MCSR1_ICACHET_SHIFT         39
#define  MCSR1_ICACHET               (1ULL << MCSR1_ICACHET_SHIFT)
#define  MCSR1_ITLBT_SHIFT           38
#define  MCSR1_ITLBT                 (1ULL << MCSR1_ITLBT_SHIFT)
#define  MCSR1_LLDBAR_SHIFT          37
#define  MCSR1_LLDBAR                (1ULL << MCSR1_LLDBAR_SHIFT)
#define  MCSR1_SCDLY_SHIFT           36
#define  MCSR1_SCDLY                 (1ULL << MCSR1_SCDLY_SHIFT)
#define  MCSR1_LLEXC_SHIFT           35
#define  MCSR1_LLEXC                 (1ULL << MCSR1_LLEXC_SHIFT)
#define  MCSR1_UCACC_SHIFT           34
#define  MCSR1_UCACC                 (1ULL << MCSR1_UCACC_SHIFT)
#define  MCSR1_SFB_SHIFT             33
#define  MCSR1_SFB                   (1ULL << MCSR1_SFB_SHIFT)
#define  MCSR1_CCDMA_SHIFT           32
#define  MCSR1_CCDMA                 (1ULL << MCSR1_CCDMA_SHIFT)
#define  MCSR1_LAMO_SHIFT            22
#define  MCSR1_LAMO                  (1ULL << MCSR1_LAMO_SHIFT)
#define  MCSR1_LSPW_SHIFT            21
#define  MCSR1_LSPW                  (1ULL << MCSR1_LSPW_SHIFT)
#define  MCSR1_LOONGARCHBT_SHIFT     20
#define  MCSR1_LOONGARCHBT           (1ULL << MCSR1_LOONGARCHBT_SHIFT)
#define  MCSR1_ARMBT_SHIFT           19
#define  MCSR1_ARMBT                 (1ULL << MCSR1_ARMBT_SHIFT)
#define  MCSR1_X86BT_SHIFT           18
#define  MCSR1_X86BT                 (1ULL << MCSR1_X86BT_SHIFT)
#define  MCSR1_LLFTPVERS_SHIFT       15
#define  MCSR1_LLFTPVERS_WIDTH       3
#define  MCSR1_LLFTPVERS             (7ULL << MCSR1_LLFTPVERS_SHIFT)
#define  MCSR1_LLFTP_SHIFT           14
#define  MCSR1_LLFTP                 (1ULL << MCSR1_LLFTP_SHIFT)
#define  MCSR1_VZVERS_SHIFT          11
#define  MCSR1_VZVERS_WIDTH          3
#define  MCSR1_VZVERS                (7ULL << MCSR1_VZVERS_SHIFT)
#define  MCSR1_VZ_SHIFT              10
#define  MCSR1_VZ                    (1ULL << MCSR1_VZ_SHIFT)
#define  MCSR1_CRYPTO_SHIFT          9
#define  MCSR1_CRYPTO                (1ULL << MCSR1_CRYPTO_SHIFT)
#define  MCSR1_COMPLEX_SHIFT         8
#define  MCSR1_COMPLEX               (1ULL << MCSR1_COMPLEX_SHIFT)
#define  MCSR1_LASX_SHIFT            7
#define  MCSR1_LASX                  (1ULL << MCSR1_LASX_SHIFT)
#define  MCSR1_LSX_SHIFT             6
#define  MCSR1_LSX                   (1ULL << MCSR1_LSX_SHIFT)
#define  MCSR1_FPVERS_SHIFT          3
#define  MCSR1_FPVERS_WIDTH          3
#define  MCSR1_FPVERS                (7ULL << MCSR1_FPVERS_SHIFT)
#define  MCSR1_FPDP_SHIFT            2
#define  MCSR1_FPDP                  (1ULL << MCSR1_FPDP_SHIFT)
#define  MCSR1_FPSP_SHIFT            1
#define  MCSR1_FPSP                  (1ULL << MCSR1_FPSP_SHIFT)
#define  MCSR1_FP_SHIFT              0
#define  MCSR1_FP                    (1ULL << MCSR1_FP_SHIFT)

#define LOONGARCH_CSR_MCSR2          0xc2 /* CPUCFG4 and CPUCFG5 */
#define  MCSR2_CCDIV_SHIFT           48
#define  MCSR2_CCDIV_WIDTH           16
#define  MCSR2_CCDIV                 (0xffffULL << MCSR2_CCDIV_SHIFT)
#define  MCSR2_CCMUL_SHIFT           32
#define  MCSR2_CCMUL_WIDTH           16
#define  MCSR2_CCMUL                 (0xffffULL << MCSR2_CCMUL_SHIFT)
#define  MCSR2_CCFREQ_WIDTH          32
#define  MCSR2_CCFREQ                (0xffffffff)
#define  CCFREQ_DEFAULT              0x5f5e100 /* 100MHZ */

#define LOONGARCH_CSR_MCSR3          0xc3 /* CPUCFG6 */
#define  MCSR3_UPM_SHIFT             14
#define  MCSR3_UPM                   (1ULL << MCSR3_UPM_SHIFT)
#define  MCSR3_PMBITS_SHIFT          8
#define  MCSR3_PMBITS_WIDTH          6
#define  MCSR3_PMBITS                (0x3fULL << MCSR3_PMBITS_SHIFT)
#define  PMBITS_DEFAULT              0x40
#define  MCSR3_PMNUM_SHIFT           4
#define  MCSR3_PMNUM_WIDTH           4
#define  MCSR3_PMNUM                 (0xfULL << MCSR3_PMNUM_SHIFT)
#define  MCSR3_PAMVER_SHIFT          1
#define  MCSR3_PAMVER_WIDTH          3
#define  MCSR3_PAMVER                (0x7ULL << MCSR3_PAMVER_SHIFT)
#define  MCSR3_PMP_SHIFT             0
#define  MCSR3_PMP                   (1ULL << MCSR3_PMP_SHIFT)

#define LOONGARCH_CSR_MCSR8          0xc8 /* CPUCFG16 and CPUCFG17 */
#define  MCSR8_L1I_SIZE_SHIFT        56
#define  MCSR8_L1I_SIZE_WIDTH        7
#define  MCSR8_L1I_SIZE              (0x7fULL << MCSR8_L1I_SIZE_SHIFT)
#define  MCSR8_L1I_IDX_SHIFT         48
#define  MCSR8_L1I_IDX_WIDTH         8
#define  MCSR8_L1I_IDX               (0xffULL << MCSR8_L1I_IDX_SHIFT)
#define  MCSR8_L1I_WAY_SHIFT         32
#define  MCSR8_L1I_WAY_WIDTH         16
#define  MCSR8_L1I_WAY               (0xffffULL << MCSR8_L1I_WAY_SHIFT)
#define  MCSR8_L3DINCL_SHIFT         16
#define  MCSR8_L3DINCL               (1ULL << MCSR8_L3DINCL_SHIFT)
#define  MCSR8_L3DPRIV_SHIFT         15
#define  MCSR8_L3DPRIV               (1ULL << MCSR8_L3DPRIV_SHIFT)
#define  MCSR8_L3DPRE_SHIFT          14
#define  MCSR8_L3DPRE                (1ULL << MCSR8_L3DPRE_SHIFT)
#define  MCSR8_L3IUINCL_SHIFT        13
#define  MCSR8_L3IUINCL              (1ULL << MCSR8_L3IUINCL_SHIFT)
#define  MCSR8_L3IUPRIV_SHIFT        12
#define  MCSR8_L3IUPRIV              (1ULL << MCSR8_L3IUPRIV_SHIFT)
#define  MCSR8_L3IUUNIFY_SHIFT       11
#define  MCSR8_L3IUUNIFY             (1ULL << MCSR8_L3IUUNIFY_SHIFT)
#define  MCSR8_L3IUPRE_SHIFT         10
#define  MCSR8_L3IUPRE               (1ULL << MCSR8_L3IUPRE_SHIFT)
#define  MCSR8_L2DINCL_SHIFT         9
#define  MCSR8_L2DINCL               (1ULL << MCSR8_L2DINCL_SHIFT)
#define  MCSR8_L2DPRIV_SHIFT         8
#define  MCSR8_L2DPRIV               (1ULL << MCSR8_L2DPRIV_SHIFT)
#define  MCSR8_L2DPRE_SHIFT          7
#define  MCSR8_L2DPRE                (1ULL << MCSR8_L2DPRE_SHIFT)
#define  MCSR8_L2IUINCL_SHIFT        6
#define  MCSR8_L2IUINCL              (1ULL << MCSR8_L2IUINCL_SHIFT)
#define  MCSR8_L2IUPRIV_SHIFT        5
#define  MCSR8_L2IUPRIV              (1ULL << MCSR8_L2IUPRIV_SHIFT)
#define  MCSR8_L2IUUNIFY_SHIFT       4
#define  MCSR8_L2IUUNIFY             (1ULL << MCSR8_L2IUUNIFY_SHIFT)
#define  MCSR8_L2IUPRE_SHIFT         3
#define  MCSR8_L2IUPRE               (1ULL << MCSR8_L2IUPRE_SHIFT)
#define  MCSR8_L1DPRE_SHIFT          2
#define  MCSR8_L1DPRE                (1ULL << MCSR8_L1DPRE_SHIFT)
#define  MCSR8_L1IUUNIFY_SHIFT       1
#define  MCSR8_L1IUUNIFY             (1ULL << MCSR8_L1IUUNIFY_SHIFT)
#define  MCSR8_L1IUPRE_SHIFT         0
#define  MCSR8_L1IUPRE               (1ULL << MCSR8_L1IUPRE_SHIFT)

#define LOONGARCH_CSR_MCSR9          0xc9 /* CPUCFG18 and CPUCFG19 */
#define  MCSR9_L2U_SIZE_SHIFT        56
#define  MCSR9_L2U_SIZE_WIDTH        7
#define  MCSR9_L2U_SIZE              (0x7fULL << MCSR9_L2U_SIZE_SHIFT)
#define  MCSR9_L2U_IDX_SHIFT         48
#define  MCSR9_L2U_IDX_WIDTH         8
#define  MCSR9_L2U_IDX               (0xffULL << MCSR9_IDX_LOG_SHIFT)
#define  MCSR9_L2U_WAY_SHIFT         32
#define  MCSR9_L2U_WAY_WIDTH         16
#define  MCSR9_L2U_WAY               (0xffffULL << MCSR9_L2U_WAY_SHIFT)
#define  MCSR9_L1D_SIZE_SHIFT        24
#define  MCSR9_L1D_SIZE_WIDTH        7
#define  MCSR9_L1D_SIZE              (0x7fULL << MCSR9_L1D_SIZE_SHIFT)
#define  MCSR9_L1D_IDX_SHIFT         16
#define  MCSR9_L1D_IDX_WIDTH         8
#define  MCSR9_L1D_IDX               (0xffULL << MCSR9_L1D_IDX_SHIFT)
#define  MCSR9_L1D_WAY_SHIFT         0
#define  MCSR9_L1D_WAY_WIDTH         16
#define  MCSR9_L1D_WAY               (0xffffULL << MCSR9_L1D_WAY_SHIFT)

#define LOONGARCH_CSR_MCSR10         0xca /* CPUCFG20 */
#define  MCSR10_L3U_SIZE_SHIFT       24
#define  MCSR10_L3U_SIZE_WIDTH       7
#define  MCSR10_L3U_SIZE             (0x7fULL << MCSR10_L3U_SIZE_SHIFT)
#define  MCSR10_L3U_IDX_SHIFT        16
#define  MCSR10_L3U_IDX_WIDTH        8
#define  MCSR10_L3U_IDX              (0xffULL << MCSR10_L3U_IDX_SHIFT)
#define  MCSR10_L3U_WAY_SHIFT        0
#define  MCSR10_L3U_WAY_WIDTH        16
#define  MCSR10_L3U_WAY              (0xffffULL << MCSR10_L3U_WAY_SHIFT)

#define LOONGARCH_CSR_MCSR24         0xf0 /* CPUCFG48 */
#define  MCSR24_RAMCG_SHIFT          3
#define  MCSR24_RAMCG                (1ULL << MCSR24_RAMCG_SHIFT)
#define  MCSR24_VFPUCG_SHIFT         2
#define  MCSR24_VFPUCG               (1ULL << MCSR24_VFPUCG_SHIFT)
#define  MCSR24_NAPEN_SHIFT          1
#define  MCSR24_NAPEN                (1ULL << MCSR24_NAPEN_SHIFT)
#define  MCSR24_MCSRLOCK_SHIFT       0
#define  MCSR24_MCSRLOCK             (1ULL << MCSR24_MCSRLOCK_SHIFT)

/* Uncached accelerate windows registers  */
#define LOONGARCH_CSR_UCAWIN         0x100 /* read only info */
#define LOONGARCH_CSR_UCAWIN0_LO     0x102 /* win0 low */
#define LOONGARCH_CSR_UCAWIN0_HI     0x103 /* win0 high */
#define LOONGARCH_CSR_UCAWIN1_LO     0x104 /* win1 low */
#define LOONGARCH_CSR_UCAWIN1_HI     0x105 /* win1 high */
#define LOONGARCH_CSR_UCAWIN2_LO     0x106 /* win2 low */
#define LOONGARCH_CSR_UCAWIN2_HI     0x107 /* win2 high */
#define LOONGARCH_CSR_UCAWIN3_LO     0x108 /* win3 low */
#define LOONGARCH_CSR_UCAWIN3_HI     0x109 /* win3 high */

/* Direct map windows registers */
#define LOONGARCH_CSR_DMWIN0         0x180 /* direct map win0: MEM & IF */
#define LOONGARCH_CSR_DMWIN1         0x181 /* direct map win1: MEM & IF */
#define LOONGARCH_CSR_DMWIN2         0x182 /* direct map win2: MEM */
#define LOONGARCH_CSR_DMWIN3         0x183 /* direct map win3: MEM */

/* Performance counter registers */
#define LOONGARCH_CSR_PERFCTRL0      0x200 /* perf event 0 config */
#define LOONGARCH_CSR_PERFCNTR0      0x201 /* perf event 0 count value */
#define LOONGARCH_CSR_PERFCTRL1      0x202 /* perf event 1 config */
#define LOONGARCH_CSR_PERFCNTR1      0x203 /* perf event 1 count value */
#define LOONGARCH_CSR_PERFCTRL2      0x204 /* perf event 2 config */
#define LOONGARCH_CSR_PERFCNTR2      0x205 /* perf event 2 count value */
#define LOONGARCH_CSR_PERFCTRL3      0x206 /* perf event 3 config */
#define LOONGARCH_CSR_PERFCNTR3      0x207 /* perf event 3 count value */
#define  CSR_PERFCTRL_PLV0           (1ULL << 16)
#define  CSR_PERFCTRL_PLV1           (1ULL << 17)
#define  CSR_PERFCTRL_PLV2           (1ULL << 18)
#define  CSR_PERFCTRL_PLV3           (1ULL << 19)
#define  CSR_PERFCTRL_IE             (1ULL << 20)
#define  CSR_PERFCTRL_EVENT          0x3ff

/* CSR register */
#define CPU_LOONGARCH_CSR    \
    uint64_t CSR_CRMD;       \
    uint64_t CSR_PRMD;       \
    uint64_t CSR_EUEN;       \
    uint64_t CSR_MISC;       \
    uint64_t CSR_ECFG;       \
    uint64_t CSR_ESTAT;      \
    uint64_t CSR_ERA;        \
    uint64_t CSR_BADV;       \
    uint64_t CSR_BADI;       \
    uint64_t CSR_EEPN;       \
    uint64_t CSR_TLBIDX;     \
    uint64_t CSR_TLBEHI;     \
    uint64_t CSR_TLBELO0;    \
    uint64_t CSR_TLBELO1;    \
    uint64_t CSR_ASID;       \
    uint64_t CSR_PGDL;       \
    uint64_t CSR_PGDH;       \
    uint64_t CSR_PGD;        \
    uint64_t CSR_PWCTL0;     \
    uint64_t CSR_PWCTL1;     \
    uint64_t CSR_STLBPGSIZE; \
    uint64_t CSR_RVACFG;     \
    uint64_t CSR_CPUID;      \
    uint64_t CSR_PRCFG1;     \
    uint64_t CSR_PRCFG2;     \
    uint64_t CSR_PRCFG3;     \
    uint64_t CSR_KS0;        \
    uint64_t CSR_KS1;        \
    uint64_t CSR_KS2;        \
    uint64_t CSR_KS3;        \
    uint64_t CSR_KS4;        \
    uint64_t CSR_KS5;        \
    uint64_t CSR_KS6;        \
    uint64_t CSR_KS7;        \
    uint64_t CSR_KS8;        \
    uint64_t CSR_TMID;       \
    uint64_t CSR_TCFG;       \
    uint64_t CSR_TVAL;       \
    uint64_t CSR_CNTC;       \
    uint64_t CSR_TINTCLR;    \
    uint64_t CSR_LLBIT;      \
    uint64_t CSR_IMPCTL1;    \
    uint64_t CSR_IMPCTL2;    \
    uint64_t CSR_GNMI;       \
    uint64_t CSR_TLBRENT;    \
    uint64_t CSR_TLBRBADV;   \
    uint64_t CSR_TLBRERA;    \
    uint64_t CSR_TLBRSAVE;   \
    uint64_t CSR_TLBRELO0;   \
    uint64_t CSR_TLBRELO1;   \
    uint64_t CSR_TLBREHI;    \
    uint64_t CSR_TLBRPRMD;   \
    uint64_t CSR_ERRCTL;     \
    uint64_t CSR_ERRINFO;    \
    uint64_t CSR_ERRINFO1;   \
    uint64_t CSR_ERRENT;     \
    uint64_t CSR_ERRERA;     \
    uint64_t CSR_ERRSAVE;    \
    uint64_t CSR_CTAG;       \
    uint64_t CSR_MCSR0;      \
    uint64_t CSR_MCSR1;      \
    uint64_t CSR_MCSR2;      \
    uint64_t CSR_MCSR3;      \
    uint64_t CSR_MCSR8;      \
    uint64_t CSR_MCSR9;      \
    uint64_t CSR_MCSR10;     \
    uint64_t CSR_MCSR24;     \
    uint64_t CSR_UCAWIN;     \
    uint64_t CSR_UCAWIN0_LO; \
    uint64_t CSR_UCAWIN0_HI; \
    uint64_t CSR_UCAWIN1_LO; \
    uint64_t CSR_UCAWIN1_HI; \
    uint64_t CSR_UCAWIN2_LO; \
    uint64_t CSR_UCAWIN2_HI; \
    uint64_t CSR_UCAWIN3_LO; \
    uint64_t CSR_UCAWIN3_HI; \
    uint64_t CSR_DMWIN0;     \
    uint64_t CSR_DMWIN1;     \
    uint64_t CSR_DMWIN2;     \
    uint64_t CSR_DMWIN3;     \
    uint64_t CSR_PERFCTRL0;  \
    uint64_t CSR_PERFCNTR0;  \
    uint64_t CSR_PERFCTRL1;  \
    uint64_t CSR_PERFCNTR1;  \
    uint64_t CSR_PERFCTRL2;  \
    uint64_t CSR_PERFCNTR2;  \
    uint64_t CSR_PERFCTRL3;  \
    uint64_t CSR_PERFCNTR3;  \

#endif
