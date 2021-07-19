/*
 * QEMU LoongArch Disassembler
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited.
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "qemu/osdep.h"
#include "disas/dis-asm.h"

#define INSNLEN 4

/* enums */
typedef enum {
    la_op_illegal = 0,
    la_op_clo_w = 1,
    la_op_clz_w = 2,
    la_op_cto_w = 3,
    la_op_ctz_w = 4,
    la_op_clo_d = 5,
    la_op_clz_d = 6,
    la_op_cto_d = 7,
    la_op_ctz_d = 8,
    la_op_revb_2h = 9,
    la_op_revb_4h = 10,
    la_op_revb_2w = 11,
    la_op_revb_d = 12,
    la_op_revh_2w = 13,
    la_op_revh_d = 14,
    la_op_bitrev_4b = 15,
    la_op_bitrev_8b = 16,
    la_op_bitrev_w = 17,
    la_op_bitrev_d = 18,
    la_op_ext_w_h = 19,
    la_op_ext_w_b = 20,
    la_op_rdtime_d = 21,
    la_op_cpucfg = 22,
    la_op_asrtle_d = 23,
    la_op_asrtgt_d = 24,
    la_op_alsl_w = 25,
    la_op_alsl_wu = 26,
    la_op_bytepick_w = 27,
    la_op_bytepick_d = 28,
    la_op_add_w = 29,
    la_op_add_d = 30,
    la_op_sub_w = 31,
    la_op_sub_d = 32,
    la_op_slt = 33,
    la_op_sltu = 34,
    la_op_maskeqz = 35,
    la_op_masknez = 36,
    la_op_nor = 37,
    la_op_and = 38,
    la_op_or = 39,
    la_op_xor = 40,
    la_op_orn = 41,
    la_op_andn = 42,
    la_op_sll_w = 43,
    la_op_srl_w = 44,
    la_op_sra_w = 45,
    la_op_sll_d = 46,
    la_op_srl_d = 47,
    la_op_sra_d = 48,
    la_op_rotr_w = 49,
    la_op_rotr_d = 50,
    la_op_mul_w = 51,
    la_op_mulh_w = 52,
    la_op_mulh_wu = 53,
    la_op_mul_d = 54,
    la_op_mulh_d = 55,
    la_op_mulh_du = 56,
    la_op_mulw_d_w = 57,
    la_op_mulw_d_wu = 58,
    la_op_div_w = 59,
    la_op_mod_w = 60,
    la_op_div_wu = 61,
    la_op_mod_wu = 62,
    la_op_div_d = 63,
    la_op_mod_d = 64,
    la_op_div_du = 65,
    la_op_mod_du = 66,
    la_op_crc_w_b_w = 67,
    la_op_crc_w_h_w = 68,
    la_op_crc_w_w_w = 69,
    la_op_crc_w_d_w = 70,
    la_op_crcc_w_b_w = 71,
    la_op_crcc_w_h_w = 72,
    la_op_crcc_w_w_w = 73,
    la_op_crcc_w_d_w = 74,
    la_op_break = 75,
    la_op_syscall = 76,
    la_op_alsl_d = 77,
    la_op_slli_w = 78,
    la_op_slli_d = 79,
    la_op_srli_w = 80,
    la_op_srli_d = 81,
    la_op_srai_w = 82,
    la_op_srai_d = 83,
    la_op_rotri_w = 84,
    la_op_rotri_d = 85,
    la_op_bstrins_w = 86,
    la_op_bstrpick_w = 87,
    la_op_bstrins_d = 88,
    la_op_bstrpick_d = 89,
    la_op_fadd_s = 90,
    la_op_fadd_d = 91,
    la_op_fsub_s = 92,
    la_op_fsub_d = 93,
    la_op_fmul_s = 94,
    la_op_fmul_d = 95,
    la_op_fdiv_s = 96,
    la_op_fdiv_d = 97,
    la_op_fmax_s = 98,
    la_op_fmax_d = 99,
    la_op_fmin_s = 100,
    la_op_fmin_d = 101,
    la_op_fmaxa_s = 102,
    la_op_fmaxa_d = 103,
    la_op_fmina_s = 104,
    la_op_fmina_d = 105,
    la_op_fscaleb_s = 106,
    la_op_fscaleb_d = 107,
    la_op_fcopysign_s = 108,
    la_op_fcopysign_d = 109,
    la_op_fabs_s = 110,
    la_op_fabs_d = 111,
    la_op_fneg_s = 112,
    la_op_fneg_d = 113,
    la_op_flogb_s = 114,
    la_op_flogb_d = 115,
    la_op_fclass_s = 116,
    la_op_fclass_d = 117,
    la_op_fsqrt_s = 118,
    la_op_fsqrt_d = 119,
    la_op_frecip_s = 120,
    la_op_frecip_d = 121,
    la_op_frsqrt_s = 122,
    la_op_frsqrt_d = 123,
    la_op_fmov_s = 124,
    la_op_fmov_d = 125,
    la_op_movgr2fr_w = 126,
    la_op_movgr2fr_d = 127,
    la_op_movgr2frh_w = 128,
    la_op_movfr2gr_s = 129,
    la_op_movfr2gr_d = 130,
    la_op_movfrh2gr_s = 131,
    la_op_movgr2fcsr = 132,
    la_op_movfcsr2gr = 133,
    la_op_movfr2cf = 134,
    la_op_movcf2fr = 135,
    la_op_movgr2cf = 136,
    la_op_movcf2gr = 137,
    la_op_fcvt_s_d = 138,
    la_op_fcvt_d_s = 139,
    la_op_ftintrm_w_s = 140,
    la_op_ftintrm_w_d = 141,
    la_op_ftintrm_l_s = 142,
    la_op_ftintrm_l_d = 143,
    la_op_ftintrp_w_s = 144,
    la_op_ftintrp_w_d = 145,
    la_op_ftintrp_l_s = 146,
    la_op_ftintrp_l_d = 147,
    la_op_ftintrz_w_s = 148,
    la_op_ftintrz_w_d = 149,
    la_op_ftintrz_l_s = 150,
    la_op_ftintrz_l_d = 151,
    la_op_ftintrne_w_s = 152,
    la_op_ftintrne_w_d = 153,
    la_op_ftintrne_l_s = 154,
    la_op_ftintrne_l_d = 155,
    la_op_ftint_w_s = 156,
    la_op_ftint_w_d = 157,
    la_op_ftint_l_s = 158,
    la_op_ftint_l_d = 159,
    la_op_ffint_s_w = 160,
    la_op_ffint_s_l = 161,
    la_op_ffint_d_w = 162,
    la_op_ffint_d_l = 163,
    la_op_frint_s = 164,
    la_op_frint_d = 165,
    la_op_slti = 166,
    la_op_sltui = 167,
    la_op_addi_w = 168,
    la_op_addi_d = 169,
    la_op_lu52i_d = 170,
    la_op_addi = 171,
    la_op_ori = 172,
    la_op_xori = 173,
    la_op_rdtimel_w = 174,
    la_op_rdtimeh_w = 175,
    la_op_fmadd_s = 176,
    la_op_fmadd_d = 177,
    la_op_fmsub_s = 178,
    la_op_fmsub_d = 179,
    la_op_fnmadd_s = 180,
    la_op_fnmadd_d = 181,
    la_op_fnmsub_s = 182,
    la_op_fnmsub_d = 183,
    la_op_fcmp_cond_s = 184,
    la_op_fcmp_cond_d = 185,
    la_op_fsel = 186,
    la_op_addu16i_d = 187,
    la_op_lu12i_w = 188,
    la_op_lu32i_d = 189,
    la_op_pcaddi = 190,
    la_op_pcalau12i = 191,
    la_op_pcaddu12i = 192,
    la_op_pcaddu18i = 193,
    la_op_ll_w = 194,
    la_op_sc_w = 195,
    la_op_ll_d = 196,
    la_op_sc_d = 197,
    la_op_ldptr_w = 198,
    la_op_stptr_w = 199,
    la_op_ldptr_d = 200,
    la_op_stptr_d = 201,
    la_op_ld_b = 202,
    la_op_ld_h = 203,
    la_op_ld_w = 204,
    la_op_ld_d = 205,
    la_op_st_b = 206,
    la_op_st_h = 207,
    la_op_st_w = 208,
    la_op_st_d = 209,
    la_op_ld_bu = 210,
    la_op_ld_hu = 211,
    la_op_ld_wu = 212,
    la_op_preld = 213,
    la_op_fld_s = 214,
    la_op_fst_s = 215,
    la_op_fld_d = 216,
    la_op_fst_d = 217,
    la_op_ldx_b = 218,
    la_op_ldx_h = 219,
    la_op_ldx_w = 220,
    la_op_ldx_d = 221,
    la_op_stx_b = 222,
    la_op_stx_h = 223,
    la_op_stx_w = 224,
    la_op_stx_d = 225,
    la_op_ldx_bu = 226,
    la_op_ldx_hu = 227,
    la_op_ldx_wu = 228,
    la_op_fldx_s = 229,
    la_op_fldx_d = 230,
    la_op_fstx_s = 231,
    la_op_fstx_d = 232,
    la_op_amswap_w = 233,
    la_op_amswap_d = 234,
    la_op_amadd_w = 235,
    la_op_amadd_d = 236,
    la_op_amand_w = 237,
    la_op_amand_d = 238,
    la_op_amor_w = 239,
    la_op_amor_d = 240,
    la_op_amxor_w = 241,
    la_op_amxor_d = 242,
    la_op_ammax_w = 243,
    la_op_ammax_d = 244,
    la_op_ammin_w = 245,
    la_op_ammin_d = 246,
    la_op_ammax_wu = 247,
    la_op_ammax_du = 248,
    la_op_ammin_wu = 249,
    la_op_ammin_du = 250,
    la_op_amswap_db_w = 251,
    la_op_amswap_db_d = 252,
    la_op_amadd_db_w = 253,
    la_op_amadd_db_d = 254,
    la_op_amand_db_w = 255,
    la_op_amand_db_d = 256,
    la_op_amor_db_w = 257,
    la_op_amor_db_d = 258,
    la_op_amxor_db_w = 259,
    la_op_amxor_db_d = 260,
    la_op_ammax_db_w = 261,
    la_op_ammax_db_d = 262,
    la_op_ammin_db_w = 263,
    la_op_ammin_db_d = 264,
    la_op_ammax_db_wu = 265,
    la_op_ammax_db_du = 266,
    la_op_ammin_db_wu = 267,
    la_op_ammin_db_du = 268,
    la_op_dbar = 269,
    la_op_ibar = 270,
    la_op_fldgt_s = 271,
    la_op_fldgt_d = 272,
    la_op_fldle_s = 273,
    la_op_fldle_d = 274,
    la_op_fstgt_s = 275,
    la_op_fstgt_d = 276,
    ls_op_fstle_s = 277,
    la_op_fstle_d = 278,
    la_op_ldgt_b = 279,
    la_op_ldgt_h = 280,
    la_op_ldgt_w = 281,
    la_op_ldgt_d = 282,
    la_op_ldle_b = 283,
    la_op_ldle_h = 284,
    la_op_ldle_w = 285,
    la_op_ldle_d = 286,
    la_op_stgt_b = 287,
    la_op_stgt_h = 288,
    la_op_stgt_w = 289,
    la_op_stgt_d = 290,
    la_op_stle_b = 291,
    la_op_stle_h = 292,
    la_op_stle_w = 293,
    la_op_stle_d = 294,
    la_op_beqz = 295,
    la_op_bnez = 296,
    la_op_bceqz = 297,
    la_op_bcnez = 298,
    la_op_jirl = 299,
    la_op_b = 300,
    la_op_bl = 301,
    la_op_beq = 302,
    la_op_bne = 303,
    la_op_blt = 304,
    la_op_bge = 305,
    la_op_bltu = 306,
    la_op_bgeu = 307,

} la_op;

typedef enum {
    la_codec_illegal,
    la_codec_empty,
    la_codec_2r,
    la_codec_2r_u5,
    la_codec_2r_u6,
    la_codec_2r_2bw,
    la_codec_2r_2bd,
    la_codec_3r,
    la_codec_3r_rd0,
    la_codec_3r_sa2,
    la_codec_3r_sa3,
    la_codec_4r,
    la_codec_r_im20,
    la_codec_2r_im16,
    la_codec_2r_im14,
    la_codec_r_im14,
    la_codec_2r_im12,
    la_codec_im5_r_im12,
    la_codec_2r_im8,
    la_codec_r_sd,
    la_codec_r_sj,
    la_codec_r_cd,
    la_codec_r_cj,
    la_codec_r_seq,
    la_codec_code,
    la_codec_whint,
    la_codec_invtlb,
    la_codec_r_ofs21,
    la_codec_cj_ofs21,
    la_codec_ofs26,
    la_codec_cond,
    la_codec_sel,

} la_codec;

#define la_fmt_illegal         "nte"
#define la_fmt_empty           "nt"
#define la_fmt_sd_rj           "ntA,1"
#define la_fmt_rd_sj           "nt0,B"
#define la_fmt_rd_rj           "nt0,1"
#define la_fmt_rj_rk           "nt1,2"
#define la_fmt_rj_seq          "nt1,x"
#define la_fmt_rd_si20         "nt0,i(x)"
#define la_fmt_rd_rj_ui5       "nt0,1,C"
#define la_fmt_rd_rj_ui6       "nt0,1.C"
#define la_fmt_rd_rj_level     "nt0,1,x"
#define la_fmt_rd_rj_msbw_lsbw "nt0,1,C,D"
#define la_fmt_rd_rj_msbd_lsbd "nt0,1,C,D"
#define la_fmt_rd_rj_si12      "nt0,1,i(x)"
#define la_fmt_hint_rj_si12    "ntE,1,i(x)"
#define la_fmt_rd_rj_csr       "nt0,1,x"
#define la_fmt_rd_csr          "nt0,x"
#define la_fmt_rd_rj_si14      "nt0,1,i(x)"
#define la_fmt_rd_rj_si16      "nt0,1,i(x)"
#define la_fmt_rd_rj_rk        "nt0,1,2"
#define la_fmt_fd_rj_rk        "nt3,1,2"
#define la_fmt_rd_rj_rk_sa2    "nt0,1,2,D"
#define la_fmt_rd_rj_rk_sa3    "nt0,1,2,D"
#define la_fmt_fd_rj           "nt3,1"
#define la_fmt_rd_fj           "nt0,4"
#define la_fmt_fd_fj           "nt3,4"
#define la_fmt_fd_fj_si12      "nt3,4,i(x)"
#define la_fmt_fcsrd_rj        "ntF,1"
#define la_fmt_rd_fcsrs        "nt0,G"
#define la_fmt_cd_fj           "ntH,4"
#define la_fmt_fd_cj           "nt3,I"
#define la_fmt_fd_fj_fk        "nt3,4,5"
#define la_fmt_code            "ntJ"
#define la_fmt_whint           "ntx"
#define la_fmt_invtlb          "ntx,1,2"
#define la_fmt_offs26          "nto(X)p"
#define la_fmt_rj_offs21       "nt1,o(X)p"
#define la_fmt_cj_offs21       "ntQ,o(X)p"
#define la_fmt_rd_rj_offs16    "nt0,1,o(X)"
#define la_fmt_rj_rd_offs16    "nt1,0,o(X)p"
#define la_fmt_s_cd_fj_fk      "K.stH,4,5"
#define la_fmt_d_cd_fj_fk      "K.dtH,4,5"
#define la_fmt_fd_fj_fk_fa     "nt3,4,5,6"
#define la_fmt_fd_fj_fk_ca     "nt3,4,5,L"
#define la_fmt_cop_rj_si12     "ntM,1,i(x)"

/* structures */
typedef struct {
    uint32_t pc;
    uint32_t insn;
    int32_t imm;
    int32_t imm2;
    uint16_t op;
    uint16_t code;
    uint8_t codec;
    uint8_t r1;
    uint8_t r2;
    uint8_t r3;
    uint8_t r4;
    uint8_t bit;
} la_decode;

typedef struct {
    const char * const name;
    const la_codec codec;
    const char * const format;
} la_opcode_data;

/* reg names */
const char * const loongarch_r_normal_name[32] = {
  "$r0", "$r1", "$r2", "$r3", "$r4", "$r5", "$r6", "$r7",
  "$r8", "$r9", "$r10", "$r11", "$r12", "$r13", "$r14", "$r15",
  "$r16", "$r17", "$r18", "$r19", "$r20", "$r21", "$r22", "$r23",
  "$r24", "$r25", "$r26", "$r27", "$r28", "$r29", "$r30", "$r31",
};

const char * const loongarch_f_normal_name[32] = {
  "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7",
  "$f8", "$f9", "$f10", "$f11", "$f12", "$f13", "$f14", "$f15",
  "$f16", "$f17", "$f18", "$f19", "$f20", "$f21", "$f22", "$f23",
  "$f24", "$f25", "$f26", "$f27", "$f28", "$f29", "$f30", "$f31",
};

const char * const loongarch_cr_normal_name[4] = {
  "$scr0", "$scr1", "$scr2", "$scr3",
};

const char * const loongarch_c_normal_name[8] = {
  "$fcc0", "$fcc1", "$fcc2", "$fcc3", "$fcc4", "$fcc5", "$fcc6", "$fcc7",
};

/* instruction data */
const  la_opcode_data opcode_data[] = {
    { "illegal", la_codec_illegal, la_fmt_illegal },
    { "clo.w", la_codec_2r, la_fmt_rd_rj },
    { "clz.w", la_codec_2r, la_fmt_rd_rj },
    { "cto.w", la_codec_2r, la_fmt_rd_rj },
    { "ctz.w", la_codec_2r, la_fmt_rd_rj },
    { "clo.d", la_codec_2r, la_fmt_rd_rj },
    { "clz.d", la_codec_2r, la_fmt_rd_rj },
    { "cto.d", la_codec_2r, la_fmt_rd_rj },
    { "ctz_d", la_codec_2r, la_fmt_rd_rj },
    { "revb.2h", la_codec_2r, la_fmt_rd_rj },
    { "revb.4h", la_codec_2r, la_fmt_rd_rj },
    { "revb.2w", la_codec_2r, la_fmt_rd_rj },
    { "revb.d", la_codec_2r, la_fmt_rd_rj },
    { "revh.2w", la_codec_2r, la_fmt_rd_rj },
    { "revh.d", la_codec_2r, la_fmt_rd_rj },
    { "bitrev.4b", la_codec_2r, la_fmt_rd_rj },
    { "bitrev.8b", la_codec_2r, la_fmt_rd_rj },
    { "bitrev.w", la_codec_2r, la_fmt_rd_rj },
    { "bitrev.d", la_codec_2r, la_fmt_rd_rj },
    { "ext.w.h", la_codec_2r, la_fmt_rd_rj },
    { "ext.w.b", la_codec_2r, la_fmt_rd_rj },
    { "rdtime.d", la_codec_2r, la_fmt_rd_rj },
    { "cpucfg", la_codec_2r, la_fmt_rd_rj },
    { "asrtle.d", la_codec_3r_rd0, la_fmt_rj_rk },
    { "asrtgt.d", la_codec_3r_rd0, la_fmt_rj_rk },
    { "alsl.w", la_codec_3r_sa2, la_fmt_rd_rj_rk_sa2 },
    { "alsl.wu", la_codec_3r_sa2, la_fmt_rd_rj_rk_sa2 },
    { "bytepick.w", la_codec_3r_sa2, la_fmt_rd_rj_rk_sa2 },
    { "bytepick.d", la_codec_3r_sa3, la_fmt_rd_rj_rk_sa3 },
    { "add.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "add.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "sub.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "sub.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "slt", la_codec_3r, la_fmt_rd_rj_rk },
    { "sltu", la_codec_3r, la_fmt_rd_rj_rk },
    { "maskeqz", la_codec_3r, la_fmt_rd_rj_rk },
    { "masknez", la_codec_3r, la_fmt_rd_rj_rk },
    { "nor", la_codec_3r, la_fmt_rd_rj_rk },
    { "and", la_codec_3r, la_fmt_rd_rj_rk },
    { "or", la_codec_3r, la_fmt_rd_rj_rk },
    { "xor", la_codec_3r, la_fmt_rd_rj_rk },
    { "orn", la_codec_3r, la_fmt_rd_rj_rk },
    { "andn", la_codec_3r, la_fmt_rd_rj_rk },
    { "sll.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "srl.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "sra.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "sll.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "srl.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "sra.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "rotr.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "rotr.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "mul.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "mulh.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "mulh.wu", la_codec_3r, la_fmt_rd_rj_rk },
    { "mul.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "mulh.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "mulh.du", la_codec_3r, la_fmt_rd_rj_rk },
    { "mulw.d.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "mulw.d.wu", la_codec_3r, la_fmt_rd_rj_rk },
    { "div.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "mod.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "div.wu", la_codec_3r, la_fmt_rd_rj_rk },
    { "mod.wu", la_codec_3r, la_fmt_rd_rj_rk },
    { "div.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "mod.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "div.du", la_codec_3r, la_fmt_rd_rj_rk },
    { "mod.du", la_codec_3r, la_fmt_rd_rj_rk },
    { "crc.w.b.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "crc.w.h.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "crc.w.w.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "crc.w.d.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "crcc.w.b.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "crcc.w.h.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "crcc.w.w.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "crcc.w.d.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "break", la_codec_code, la_fmt_code },
    { "syscall", la_codec_code, la_fmt_code },
    { "alsl.d", la_codec_3r_sa2, la_fmt_rd_rj_rk_sa2 },
    { "slli.w", la_codec_2r_u5, la_fmt_rd_rj_ui5 },
    { "slli.d", la_codec_2r_u6, la_fmt_rd_rj_ui6 },
    { "srli.w", la_codec_2r_u5, la_fmt_rd_rj_ui5 },
    { "srli.d", la_codec_2r_u6, la_fmt_rd_rj_ui6 },
    { "srai.w", la_codec_2r_u5, la_fmt_rd_rj_ui5 },
    { "srai.d", la_codec_2r_u6, la_fmt_rd_rj_ui6 },
    { "rotri.w", la_codec_2r_u5, la_fmt_rd_rj_ui5 },
    { "rotri.d", la_codec_2r_u6, la_fmt_rd_rj_ui6 },
    { "bstrins.w", la_codec_2r_2bw, la_fmt_rd_rj_msbw_lsbw },
    { "bstrpick.w", la_codec_2r_2bw, la_fmt_rd_rj_msbw_lsbw },
    { "bstrins.d", la_codec_2r_2bd, la_fmt_rd_rj_msbd_lsbd },
    { "bstrpick.d", la_codec_2r_2bd, la_fmt_rd_rj_msbd_lsbd },
    { "fadd.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fadd.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fsub.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fsub.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmul.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmul.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fdiv.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fdiv.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmax.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmax.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmin.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmin.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmaxa.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmaxa.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmina.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fmina.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fscaleb.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fscaleb.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fcopysign.s", la_codec_3r, la_fmt_fd_fj_fk },
    { "fcopysign.d", la_codec_3r, la_fmt_fd_fj_fk },
    { "fabs.s", la_codec_2r, la_fmt_fd_fj },
    { "fabs.d", la_codec_2r, la_fmt_fd_fj },
    { "fneg.s", la_codec_2r, la_fmt_fd_fj },
    { "fneg.d", la_codec_2r, la_fmt_fd_fj },
    { "flogb.s", la_codec_2r, la_fmt_fd_fj },
    { "flogb.d", la_codec_2r, la_fmt_fd_fj },
    { "fclass.s", la_codec_2r, la_fmt_fd_fj },
    { "fclass.d", la_codec_2r, la_fmt_fd_fj },
    { "fsqrt.s", la_codec_2r, la_fmt_fd_fj },
    { "fsqrt.d", la_codec_2r, la_fmt_fd_fj },
    { "frecip.s", la_codec_2r, la_fmt_fd_fj },
    { "frecip.d", la_codec_2r, la_fmt_fd_fj },
    { "frsqrt.s", la_codec_2r, la_fmt_fd_fj },
    { "frsqrt.d", la_codec_2r, la_fmt_fd_fj },
    { "fmov.s", la_codec_2r, la_fmt_fd_fj },
    { "fmov.d", la_codec_2r, la_fmt_fd_fj },
    { "movgr2fr.w", la_codec_2r, la_fmt_fd_rj },
    { "movgr2fr.d", la_codec_2r, la_fmt_fd_rj },
    { "movgr2frh.w", la_codec_2r, la_fmt_fd_rj },
    { "movfr2gr.s", la_codec_2r, la_fmt_rd_fj },
    { "movfr2gr.d", la_codec_2r, la_fmt_rd_fj },
    { "movfrh2gr.s", la_codec_2r, la_fmt_rd_fj },
    { "movgr2fcsr", la_codec_2r, la_fmt_fcsrd_rj },
    { "movfcsr2gr", la_codec_2r, la_fmt_rd_fcsrs },
    { "movfr2cf", la_codec_r_cd, la_fmt_cd_fj },
    { "movcf2fr", la_codec_r_cj, la_fmt_fd_cj },
    { "movgr2cf", la_codec_r_cd, la_fmt_cd_fj },
    { "movcf2gr", la_codec_r_cj, la_fmt_fd_cj },
    { "fcvt.s.d", la_codec_2r, la_fmt_fd_fj },
    { "fcvt.d.s", la_codec_2r, la_fmt_fd_fj },
    { "ftintrm.w.s", la_codec_2r, la_fmt_fd_fj },
    { "ftintrm.w.d", la_codec_2r, la_fmt_fd_fj },
    { "ftintrm.l.s", la_codec_2r, la_fmt_fd_fj },
    { "ftintrm.l.d", la_codec_2r, la_fmt_fd_fj },
    { "ftintrp.w.s", la_codec_2r, la_fmt_fd_fj },
    { "ftintrp.w.d", la_codec_2r, la_fmt_fd_fj },
    { "ftintrp.l.s", la_codec_2r, la_fmt_fd_fj },
    { "ftintrp.l.d", la_codec_2r, la_fmt_fd_fj },
    { "ftintrz.w.s", la_codec_2r, la_fmt_fd_fj },
    { "ftintrz.w.d", la_codec_2r, la_fmt_fd_fj },
    { "ftintrz.l.s", la_codec_2r, la_fmt_fd_fj },
    { "ftintrz.l.d", la_codec_2r, la_fmt_fd_fj },
    { "ftintrne.w.s", la_codec_2r, la_fmt_fd_fj },
    { "ftintrne.w.d", la_codec_2r, la_fmt_fd_fj },
    { "ftintrne.l.s", la_codec_2r, la_fmt_fd_fj },
    { "ftintrne.l.d", la_codec_2r, la_fmt_fd_fj },
    { "ftint.w.s", la_codec_2r, la_fmt_fd_fj },
    { "ftint.w.d", la_codec_2r, la_fmt_fd_fj },
    { "ftint.l.s", la_codec_2r, la_fmt_fd_fj },
    { "ftint.l.d", la_codec_2r, la_fmt_fd_fj },
    { "ffint.s.w", la_codec_2r, la_fmt_fd_fj },
    { "ffint.s.l", la_codec_2r, la_fmt_fd_fj },
    { "ffint.d.w", la_codec_2r, la_fmt_fd_fj },
    { "ffint.d.l", la_codec_2r, la_fmt_fd_fj },
    { "frint.s", la_codec_2r, la_fmt_fd_fj },
    { "frint.d", la_codec_2r, la_fmt_fd_fj },
    { "slti", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "sltui", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "addi.w", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "addi.d", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "lu52i.d", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "addi", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "ori", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "xori", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "rdtimel.w", la_codec_2r, la_fmt_rd_rj },
    { "rdtimeh.w", la_codec_2r, la_fmt_rd_rj },
    { "fmadd.s", la_codec_4r, la_fmt_fd_fj_fk_fa },
    { "fmadd.d", la_codec_4r, la_fmt_fd_fj_fk_fa },
    { "fmsub.s", la_codec_4r, la_fmt_fd_fj_fk_fa },
    { "fmsub.d", la_codec_4r, la_fmt_fd_fj_fk_fa },
    { "fnmadd.s", la_codec_4r, la_fmt_fd_fj_fk_fa },
    { "fnmadd.d", la_codec_4r, la_fmt_fd_fj_fk_fa },
    { "fnmsub.s", la_codec_4r, la_fmt_fd_fj_fk_fa },
    { "fnmsub.d", la_codec_4r, la_fmt_fd_fj_fk_fa },
    { "fcmp.cond.s", la_codec_cond, la_fmt_s_cd_fj_fk },
    { "fcmp.cond.d", la_codec_cond, la_fmt_d_cd_fj_fk },
    { "fsel", la_codec_sel, la_fmt_fd_fj_fk_ca },
    { "addu16i.d", la_codec_2r_im16, la_fmt_rd_rj_si16 },
    { "lu12i.w", la_codec_r_im20, la_fmt_rd_si20 },
    { "lu32i.d", la_codec_r_im20, la_fmt_rd_si20 },
    { "pcaddi", la_codec_r_im20, la_fmt_rd_si20 },
    { "pcalau12i", la_codec_r_im20, la_fmt_rd_si20 },
    { "pcaddu12i", la_codec_r_im20, la_fmt_rd_si20 },
    { "pcaddu18i", la_codec_r_im20, la_fmt_rd_si20 },
    { "ll.w", la_codec_2r_im14, la_fmt_rd_rj_si14 },
    { "sc.w", la_codec_2r_im14, la_fmt_rd_rj_si14 },
    { "ll.d", la_codec_2r_im14, la_fmt_rd_rj_si14 },
    { "sc.d", la_codec_2r_im14, la_fmt_rd_rj_si14 },
    { "ldptr.w", la_codec_2r_im14, la_fmt_rd_rj_si14 },
    { "stptr.w", la_codec_2r_im14, la_fmt_rd_rj_si14 },
    { "ldptr.d", la_codec_2r_im14, la_fmt_rd_rj_si14 },
    { "stptr.d", la_codec_2r_im14, la_fmt_rd_rj_si14 },
    { "ld.b", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "ld.h", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "ld.w", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "ld.d", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "st.b", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "st.h", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "st.w", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "st.d", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "ld.bu", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "ld.hu", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "ld.wu", la_codec_2r_im12, la_fmt_rd_rj_si12 },
    { "preld", la_codec_2r_im12, la_fmt_hint_rj_si12 },
    { "fld.s", la_codec_2r_im12, la_fmt_fd_fj_si12 },
    { "fst.s", la_codec_2r_im12, la_fmt_fd_fj_si12 },
    { "fld.d", la_codec_2r_im12, la_fmt_fd_fj_si12 },
    { "fst.d", la_codec_2r_im12, la_fmt_fd_fj_si12 },
    { "ldx.b", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldx.h", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldx.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldx.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "stx.b", la_codec_3r, la_fmt_rd_rj_rk },
    { "stx.h", la_codec_3r, la_fmt_rd_rj_rk },
    { "stx.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "stx.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldx.bu", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldx.hu", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldx.wu", la_codec_3r, la_fmt_rd_rj_rk },
    { "fldx.s", la_codec_3r, la_fmt_fd_rj_rk },
    { "fldx.d", la_codec_3r, la_fmt_fd_rj_rk },
    { "fstx.s", la_codec_3r, la_fmt_fd_rj_rk },
    { "fstx.d", la_codec_3r, la_fmt_fd_rj_rk },
    { "amswap.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amswap.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "amadd.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amadd.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "amand.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amand.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "amor.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amor.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "amxor.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amxor.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammax.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammax.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammin.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammin.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammax.wu", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammax.du", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammin.wu", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammin.du", la_codec_3r, la_fmt_rd_rj_rk },
    { "amswap.db.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amswap.db.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "amadd.db.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amadd.db.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "amand.db.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amand.db.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "amor.db.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amor.db.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "amxor.db.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "amxor.db.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammax.db.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammax.db.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammin.db.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammin.db.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammax.db.wu", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammax.db.du", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammin.db.wu", la_codec_3r, la_fmt_rd_rj_rk },
    { "ammin.db.du", la_codec_3r, la_fmt_rd_rj_rk },
    { "dbar", la_codec_whint, la_fmt_whint },
    { "ibar", la_codec_whint, la_fmt_whint },
    { "fldgt.s", la_codec_3r, la_fmt_fd_rj_rk },
    { "fldgt.d", la_codec_3r, la_fmt_fd_rj_rk },
    { "fldle.s", la_codec_3r, la_fmt_fd_rj_rk },
    { "fldle.d", la_codec_3r, la_fmt_fd_rj_rk },
    { "fstgt.s", la_codec_3r, la_fmt_fd_rj_rk },
    { "fstgt.d", la_codec_3r, la_fmt_fd_rj_rk },
    { "fstle.s", la_codec_3r, la_fmt_fd_rj_rk },
    { "fstle.d", la_codec_3r, la_fmt_fd_rj_rk },
    { "ldgt.b", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldgt.h", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldgt.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldgt.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldle.b", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldle.h", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldle.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "ldle.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "stgt.b", la_codec_3r, la_fmt_rd_rj_rk },
    { "stgt.h", la_codec_3r, la_fmt_rd_rj_rk },
    { "stgt.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "stgt.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "stle.b", la_codec_3r, la_fmt_rd_rj_rk },
    { "stle.h", la_codec_3r, la_fmt_rd_rj_rk },
    { "stle.w", la_codec_3r, la_fmt_rd_rj_rk },
    { "stle.d", la_codec_3r, la_fmt_rd_rj_rk },
    { "beqz", la_codec_r_ofs21, la_fmt_rj_offs21 },
    { "bnez", la_codec_r_ofs21, la_fmt_rj_offs21 },
    { "bceqz", la_codec_cj_ofs21, la_fmt_cj_offs21 },
    { "bcnez", la_codec_cj_ofs21, la_fmt_cj_offs21 },
    { "jirl", la_codec_2r_im16, la_fmt_rd_rj_offs16 },
    { "b", la_codec_ofs26, la_fmt_offs26 },
    { "bl", la_codec_ofs26, la_fmt_offs26 },
    { "beq", la_codec_2r_im16, la_fmt_rj_rd_offs16 },
    { "bne", la_codec_2r_im16, la_fmt_rj_rd_offs16 },
    { "blt", la_codec_2r_im16, la_fmt_rj_rd_offs16 },
    { "bge", la_codec_2r_im16, la_fmt_rj_rd_offs16 },
    { "bltu", la_codec_2r_im16, la_fmt_rj_rd_offs16 },
    { "bgeu", la_codec_2r_im16, la_fmt_rj_rd_offs16 },

};


/* decode opcode */
static void decode_insn_opcode(la_decode *dec)
{
    uint32_t insn = dec->insn;
    uint16_t op = la_op_illegal;
    switch ((insn >> 26) & 0x3f) {
    case 0x0:
        switch ((insn >> 22) & 0xf) {
        case 0x0:
            switch ((insn >> 18) & 0xf) {
            case 0x0:
                switch ((insn >> 15) & 0x7) {
                case 0x0:
                    switch ((insn >> 10) & 0x1f) {
                    case 0x4:
                        op = la_op_clo_w;
                        break;
                    case 0x5:
                        op = la_op_clz_w;
                        break;
                    case 0x6:
                        op = la_op_cto_w;
                        break;
                    case 0x7:
                        op = la_op_ctz_w;
                        break;
                    case 0x8:
                        op = la_op_clo_d;
                        break;
                    case 0x9:
                        op = la_op_clz_d;
                        break;
                    case 0xa:
                        op = la_op_cto_d;
                        break;
                    case 0xb:
                        op = la_op_ctz_d;
                        break;
                    case 0xc:
                        op = la_op_revb_2h;
                        break;
                    case 0xd:
                        op = la_op_revb_4h;
                        break;
                    case 0xe:
                        op = la_op_revb_2w;
                        break;
                    case 0xf:
                        op = la_op_revb_d;
                        break;
                    case 0x10:
                        op = la_op_revh_2w;
                        break;
                    case 0x11:
                        op = la_op_revh_d;
                        break;
                    case 0x12:
                        op = la_op_bitrev_4b;
                        break;
                    case 0x13:
                        op = la_op_bitrev_8b;
                        break;
                    case 0x14:
                        op = la_op_bitrev_w;
                        break;
                    case 0x15:
                        op = la_op_bitrev_d;
                        break;
                    case 0x16:
                        op = la_op_ext_w_h;
                        break;
                    case 0x17:
                        op = la_op_ext_w_b;
                        break;
                    case 0x18:
                        op = la_op_rdtimel_w;
                        break;
                    case 0x19:
                        op = la_op_rdtimeh_w;
                        break;
                    case 0x1a:
                        op = la_op_rdtime_d;
                        break;
                    case 0x1b:
                        op = la_op_cpucfg;
                        break;
                    }
                    break;
                case 0x2:
                    switch (insn & 0x0000001f) {
                    case 0x00000000:
                        op = la_op_asrtle_d;
                        break;
                    }
                    break;
                case 0x3:
                    switch (insn & 0x0000001f) {
                    case 0x00000000:
                        op = la_op_asrtgt_d;
                        break;
                    }
                    break;
                }
                break;
            case 0x1:
                switch ((insn >> 17) & 0x1) {
                case 0x0:
                    op = la_op_alsl_w;
                    break;
                case 0x1:
                    op = la_op_alsl_wu;
                    break;
                }
                break;
            case 0x2:
                switch ((insn >> 17) & 0x1) {
                case 0x0:
                    op = la_op_bytepick_w;
                    break;
                }
                break;
            case 0x3:
                op = la_op_bytepick_d;
                break;
            case 0x4:
                switch ((insn >> 15) & 0x7) {
                case 0x0:
                    op = la_op_add_w;
                    break;
                case 0x1:
                    op = la_op_add_d;
                    break;
                case 0x2:
                    op = la_op_sub_w;
                    break;
                case 0x3:
                    op = la_op_sub_d;
                    break;
                case 0x4:
                    op = la_op_slt;
                    break;
                case 0x5:
                    op = la_op_sltu;
                    break;
                case 0x6:
                    op = la_op_maskeqz;
                    break;
                case 0x7:
                    op = la_op_masknez;
                    break;
                }
                break;
            case 0x5:
                switch ((insn >> 15) & 0x7) {
                case 0x0:
                    op = la_op_nor;
                    break;
                case 0x1:
                    op = la_op_and;
                    break;
                case 0x2:
                    op = la_op_or;
                    break;
                case 0x3:
                    op = la_op_xor;
                    break;
                case 0x4:
                    op = la_op_orn;
                    break;
                case 0x5:
                    op = la_op_andn;
                    break;
                case 0x6:
                    op = la_op_sll_w;
                    break;
                case 0x7:
                    op = la_op_srl_w;
                    break;
                }
                break;
            case 0x6:
                switch ((insn >> 15) & 0x7) {
                case 0x0:
                    op = la_op_sra_w;
                    break;
                case 0x1:
                    op = la_op_sll_d;
                    break;
                case 0x2:
                    op = la_op_srl_d;
                    break;
                case 0x3:
                    op = la_op_sra_d;
                    break;
                case 0x6:
                    op = la_op_rotr_w;
                    break;
                case 0x7:
                    op = la_op_rotr_d;
                    break;
                }
                break;
            case 0x7:
                switch ((insn >> 15) & 0x7) {
                case 0x0:
                    op = la_op_mul_w;
                    break;
                case 0x1:
                    op = la_op_mulh_w;
                    break;
                case 0x2:
                    op = la_op_mulh_wu;
                    break;
                case 0x3:
                    op = la_op_mul_d;
                    break;
                case 0x4:
                    op = la_op_mulh_d;
                    break;
                case 0x5:
                    op = la_op_mulh_du;
                    break;
                case 0x6:
                    op = la_op_mulw_d_w;
                    break;
                case 0x7:
                    op = la_op_mulw_d_wu;
                    break;
                }
                break;
            case 0x8:
                switch ((insn >> 15) & 0x7) {
                case 0x0:
                    op = la_op_div_w;
                    break;
                case 0x1:
                    op = la_op_mod_w;
                    break;
                case 0x2:
                    op = la_op_div_wu;
                    break;
                case 0x3:
                    op = la_op_mod_wu;
                    break;
                case 0x4:
                    op = la_op_div_d;
                    break;
                case 0x5:
                    op = la_op_mod_d;
                    break;
                case 0x6:
                    op = la_op_div_du;
                    break;
                case 0x7:
                    op = la_op_mod_du;
                    break;
                }
                break;
            case 0x9:
                switch ((insn >> 15) & 0x7) {
                case 0x0:
                    op = la_op_crc_w_b_w;
                    break;
                case 0x1:
                    op = la_op_crc_w_h_w;
                    break;
                case 0x2:
                    op = la_op_crc_w_w_w;
                    break;
                case 0x3:
                    op = la_op_crc_w_d_w;
                    break;
                case 0x4:
                    op = la_op_crcc_w_b_w;
                    break;
                case 0x5:
                    op = la_op_crcc_w_h_w;
                    break;
                case 0x6:
                    op = la_op_crcc_w_w_w;
                    break;
                case 0x7:
                    op = la_op_crcc_w_d_w;
                    break;
                }
                break;
            case 0xa:
                switch ((insn >> 15) & 0x7) {
                case 0x4:
                    op = la_op_break;
                    break;
                case 0x6:
                    op = la_op_syscall;
                    break;
                }
                break;
            case 0xb:
                switch ((insn >> 17) & 0x1) {
                case 0x0:
                    op = la_op_alsl_d;
                    break;
                }
                break;
            }
            break;
        case 0x1:
            switch ((insn >> 21) & 0x1) {
            case 0x0:
                switch ((insn >> 16) & 0x1f) {
                case 0x0:
                    switch ((insn >> 15) & 0x1) {
                    case 0x1:
                        op = la_op_slli_w;
                        break;
                    }
                    break;
                case 0x1:
                    op = la_op_slli_d;
                    break;
                case 0x4:
                    switch ((insn >> 15) & 0x1) {
                    case 0x1:
                        op = la_op_srli_w;
                        break;
                    }
                    break;
                case 0x5:
                    op = la_op_srli_d;
                    break;
                case 0x8:
                    switch ((insn >> 15) & 0x1) {
                    case 0x1:
                        op = la_op_srai_w;
                        break;
                    }
                    break;
                case 0x9:
                    op = la_op_srai_d;
                    break;
                case 0xc:
                    switch ((insn >> 15) & 0x1) {
                    case 0x1:
                        op = la_op_rotri_w;
                        break;
                    }
                    break;
                case 0xd:
                    op = la_op_rotri_d;
                    break;
                }
                break;
            case 0x1:
                switch ((insn >> 15) & 0x1) {
                case 0x0:
                    op = la_op_bstrins_w;
                    break;
                case 0x1:
                    op = la_op_bstrpick_w;
                    break;
                }
                break;
            }
            break;
        case 0x2:
            op = la_op_bstrins_d;
            break;
        case 0x3:
            op = la_op_bstrpick_d;
            break;
        case 0x4:
            switch ((insn >> 15) & 0x7f) {
            case 0x1:
                op = la_op_fadd_s;
                break;
            case 0x2:
                op = la_op_fadd_d;
                break;
            case 0x5:
                op = la_op_fsub_s;
                break;
            case 0x6:
                op = la_op_fsub_d;
                break;
            case 0x9:
                op = la_op_fmul_s;
                break;
            case 0xa:
                op = la_op_fmul_d;
                break;
            case 0xd:
                op = la_op_fdiv_s;
                break;
            case 0xe:
                op = la_op_fdiv_d;
                break;
            case 0x11:
                op = la_op_fmax_s;
                break;
            case 0x12:
                op = la_op_fmax_d;
                break;
            case 0x15:
                op = la_op_fmin_s;
                break;
            case 0x16:
                op = la_op_fmin_d;
                break;
            case 0x19:
                op = la_op_fmaxa_s;
                break;
            case 0x1a:
                op = la_op_fmaxa_d;
                break;
            case 0x1d:
                op = la_op_fmina_s;
                break;
            case 0x1e:
                op = la_op_fmina_d;
                break;
            case 0x21:
                op = la_op_fscaleb_s;
                break;
            case 0x22:
                op = la_op_fscaleb_d;
                break;
            case 0x25:
                op = la_op_fcopysign_s;
                break;
            case 0x26:
                op = la_op_fcopysign_d;
                break;
            case 0x28:
                switch ((insn >> 10) & 0x1f) {
                case 0x1:
                    op = la_op_fabs_s;
                    break;
                case 0x2:
                    op = la_op_fabs_d;
                    break;
                case 0x5:
                    op = la_op_fneg_s;
                    break;
                case 0x6:
                    op = la_op_fneg_d;
                    break;
                case 0x9:
                    op = la_op_flogb_s;
                    break;
                case 0xa:
                    op = la_op_flogb_d;
                    break;
                case 0xd:
                    op = la_op_fclass_s;
                    break;
                case 0xe:
                    op = la_op_fclass_d;
                    break;
                case 0x11:
                    op = la_op_fsqrt_s;
                    break;
                case 0x12:
                    op = la_op_fsqrt_d;
                    break;
                case 0x15:
                    op = la_op_frecip_s;
                    break;
                case 0x16:
                    op = la_op_frecip_d;
                    break;
                case 0x19:
                    op = la_op_frsqrt_s;
                    break;
                case 0x1a:
                    op = la_op_frsqrt_d;
                    break;
                }
                break;
            case 0x29:
                switch ((insn >> 10) & 0x1f) {
                case 0x5:
                    op = la_op_fmov_s;
                    break;
                case 0x6:
                    op = la_op_fmov_d;
                    break;
                case 0x9:
                    op = la_op_movgr2fr_w;
                    break;
                case 0xa:
                    op = la_op_movgr2fr_d;
                    break;
                case 0xb:
                    op = la_op_movgr2frh_w;
                    break;
                case 0xd:
                    op = la_op_movfr2gr_s;
                    break;
                case 0xe:
                    op = la_op_movfr2gr_d;
                    break;
                case 0xf:
                    op = la_op_movfrh2gr_s;
                    break;
                case 0x10:
                    op = la_op_movgr2fcsr;
                    break;
                case 0x12:
                    op = la_op_movfcsr2gr;
                    break;
                case 0x14:
                    switch ((insn >> 3) & 0x3) {
                    case 0x0:
                        op = la_op_movfr2cf;
                        break;
                    }
                    break;
                case 0x15:
                    switch ((insn >> 8) & 0x3) {
                    case 0x0:
                        op = la_op_movcf2fr;
                        break;
                    }
                    break;
                case 0x16:
                    switch ((insn >> 3) & 0x3) {
                    case 0x0:
                        op = la_op_movgr2cf;
                        break;
                    }
                    break;
                case 0x17:
                    switch ((insn >> 8) & 0x3) {
                    case 0x0:
                        op = la_op_movcf2gr;
                        break;
                    }
                    break;
                }
                break;
            case 0x32:
                switch ((insn >> 10) & 0x1f) {
                case 0x6:
                    op = la_op_fcvt_s_d;
                    break;
                case 0x9:
                    op = la_op_fcvt_d_s;
                    break;
                }
                break;
            case 0x34:
                switch ((insn >> 10) & 0x1f) {
                case 0x1:
                    op = la_op_ftintrm_w_s;
                    break;
                case 0x2:
                    op = la_op_ftintrm_w_d;
                    break;
                case 0x9:
                    op = la_op_ftintrm_l_s;
                    break;
                case 0xa:
                    op = la_op_ftintrm_l_d;
                    break;
                case 0x11:
                    op = la_op_ftintrp_w_s;
                    break;
                case 0x12:
                    op = la_op_ftintrp_w_d;
                    break;
                case 0x19:
                    op = la_op_ftintrp_l_s;
                    break;
                case 0x1a:
                    op = la_op_ftintrp_l_d;
                    break;
                }
                break;
            case 0x35:
                switch ((insn >> 10) & 0x1f) {
                case 0x1:
                    op = la_op_ftintrz_w_s;
                    break;
                case 0x2:
                    op = la_op_ftintrz_w_d;
                    break;
                case 0x9:
                    op = la_op_ftintrz_l_s;
                    break;
                case 0xa:
                    op = la_op_ftintrz_l_d;
                    break;
                case 0x11:
                    op = la_op_ftintrne_w_s;
                    break;
                case 0x12:
                    op = la_op_ftintrne_w_d;
                    break;
                case 0x19:
                    op = la_op_ftintrne_l_s;
                    break;
                case 0x1a:
                    op = la_op_ftintrne_l_d;
                    break;
                }
                break;
            case 0x36:
                switch ((insn >> 10) & 0x1f) {
                case 0x1:
                    op = la_op_ftint_w_s;
                    break;
                case 0x2:
                    op = la_op_ftint_w_d;
                    break;
                case 0x9:
                    op = la_op_ftint_l_s;
                    break;
                case 0xa:
                    op = la_op_ftint_l_d;
                    break;
                }
                break;
            case 0x3a:
                switch ((insn >> 10) & 0x1f) {
                case 0x4:
                    op = la_op_ffint_s_w;
                    break;
                case 0x6:
                    op = la_op_ffint_s_l;
                    break;
                case 0x8:
                    op = la_op_ffint_d_w;
                    break;
                case 0xa:
                    op = la_op_ffint_d_l;
                    break;
                }
                break;
            case 0x3c:
                switch ((insn >> 10) & 0x1f) {
                case 0x11:
                    op = la_op_frint_s;
                    break;
                case 0x12:
                    op = la_op_frint_d;
                    break;
                }
                break;
            }
            break;
        case 0x8:
            op = la_op_slti;
            break;
        case 0x9:
            op = la_op_sltui;
            break;
        case 0xa:
            op = la_op_addi_w;
            break;
        case 0xb:
            op = la_op_addi_d;
            break;
        case 0xc:
            op = la_op_lu52i_d;
            break;
        case 0xd:
            op = la_op_addi;
            break;
        case 0xe:
            op = la_op_ori;
            break;
        case 0xf:
            op = la_op_xori;
            break;
        }
        break;
    case 0x2:
        switch ((insn >> 20) & 0x3f) {
        case 0x1:
            op = la_op_fmadd_s;
            break;
        case 0x2:
            op = la_op_fmadd_d;
            break;
        case 0x5:
            op = la_op_fmsub_s;
            break;
        case 0x6:
            op = la_op_fmsub_d;
            break;
        case 0x9:
            op = la_op_fnmadd_s;
            break;
        case 0xa:
            op = la_op_fnmadd_d;
            break;
        case 0xd:
            op = la_op_fnmsub_s;
            break;
        case 0xe:
            op = la_op_fnmsub_d;
            break;
        }
        break;
    case 0x3:
        switch ((insn >> 20) & 0x3f) {
        case 0x1:
            switch ((insn >> 3) & 0x3) {
            case 0x0:
                op = la_op_fcmp_cond_s;
                break;
            }
            break;
        case 0x2:
            switch ((insn >> 3) & 0x3) {
            case 0x0:
                op = la_op_fcmp_cond_d;
                break;
            }
            break;
        case 0x10:
            switch ((insn >> 18) & 0x3) {
            case 0x0:
                op = la_op_fsel;
                break;
            }
            break;
        }
        break;
    case 0x4:
        op = la_op_addu16i_d;
        break;
    case 0x5:
        switch ((insn >> 25) & 0x1) {
        case 0x0:
            op = la_op_lu12i_w;
            break;
        case 0x1:
            op = la_op_lu32i_d;
            break;
        }
        break;
    case 0x6:
        switch ((insn >> 25) & 0x1) {
        case 0x0:
            op = la_op_pcaddi;
            break;
        case 0x1:
            op = la_op_pcalau12i;
            break;
        }
        break;
    case 0x7:
        switch ((insn >> 25) & 0x1) {
        case 0x0:
            op = la_op_pcaddu12i;
            break;
        case 0x1:
            op = la_op_pcaddu18i;
            break;
        }
        break;
    case 0x8:
        switch ((insn >> 24) & 0x3) {
        case 0x0:
            op = la_op_ll_w;
            break;
        case 0x1:
            op = la_op_sc_w;
            break;
        case 0x2:
            op = la_op_ll_d;
            break;
        case 0x3:
            op = la_op_sc_d;
            break;
        }
        break;
    case 0x9:
        switch ((insn >> 24) & 0x3) {
        case 0x0:
            op = la_op_ldptr_w;
            break;
        case 0x1:
            op = la_op_stptr_w;
            break;
        case 0x2:
            op = la_op_ldptr_d;
            break;
        case 0x3:
            op = la_op_stptr_d;
            break;
        }
        break;
    case 0xa:
        switch ((insn >> 22) & 0xf) {
        case 0x0:
            op = la_op_ld_b;
            break;
        case 0x1:
            op = la_op_ld_h;
            break;
        case 0x2:
            op = la_op_ld_w;
            break;
        case 0x3:
            op = la_op_ld_d;
            break;
        case 0x4:
            op = la_op_st_b;
            break;
        case 0x5:
            op = la_op_st_h;
            break;
        case 0x6:
            op = la_op_st_w;
            break;
        case 0x7:
            op = la_op_st_d;
            break;
        case 0x8:
            op = la_op_ld_bu;
            break;
        case 0x9:
            op = la_op_ld_hu;
            break;
        case 0xa:
            op = la_op_ld_wu;
            break;
        case 0xb:
            op = la_op_preld;
            break;
        case 0xc:
            op = la_op_fld_s;
            break;
        case 0xd:
            op = la_op_fst_s;
            break;
        case 0xe:
            op = la_op_fld_d;
            break;
        case 0xf:
            op = la_op_fst_d;
            break;
        }
        break;
    case 0xe:
        switch ((insn >> 15) & 0x7ff) {
        case 0x0:
            op = la_op_ldx_b;
            break;
        case 0x8:
            op = la_op_ldx_h;
            break;
        case 0x10:
            op = la_op_ldx_w;
            break;
        case 0x18:
            op = la_op_ldx_d;
            break;
        case 0x20:
            op = la_op_stx_b;
            break;
        case 0x28:
            op = la_op_stx_h;
            break;
        case 0x30:
            op = la_op_stx_w;
            break;
        case 0x38:
            op = la_op_stx_d;
            break;
        case 0x40:
            op = la_op_ldx_bu;
            break;
        case 0x48:
            op = la_op_ldx_hu;
            break;
        case 0x50:
            op = la_op_ldx_wu;
            break;
        case 0x60:
            op = la_op_fldx_s;
            break;
        case 0x68:
            op = la_op_fldx_d;
            break;
        case 0x70:
            op = la_op_fstx_s;
            break;
        case 0x78:
            op = la_op_fstx_d;
            break;
        case 0xc0:
            op = la_op_amswap_w;
            break;
        case 0xc1:
            op = la_op_amswap_d;
            break;
        case 0xc2:
            op = la_op_amadd_w;
            break;
        case 0xc3:
            op = la_op_amadd_d;
            break;
        case 0xc4:
            op = la_op_amand_w;
            break;
        case 0xc5:
            op = la_op_amand_d;
            break;
        case 0xc6:
            op = la_op_amor_w;
            break;
        case 0xc7:
            op = la_op_amor_d;
            break;
        case 0xc8:
            op = la_op_amxor_w;
            break;
        case 0xc9:
            op = la_op_amxor_d;
            break;
        case 0xca:
            op = la_op_ammax_w;
            break;
        case 0xcb:
            op = la_op_ammax_d;
            break;
        case 0xcc:
            op = la_op_ammin_w;
            break;
        case 0xcd:
            op = la_op_ammin_d;
            break;
        case 0xce:
            op = la_op_ammax_wu;
            break;
        case 0xcf:
            op = la_op_ammax_du;
            break;
        case 0xd0:
            op = la_op_ammin_wu;
             break;
        case 0xd1:
            op = la_op_ammin_du;
            break;
        case 0xd2:
            op = la_op_amswap_db_w;
            break;
        case 0xd3:
            op = la_op_amswap_db_d;
            break;
        case 0xd4:
            op = la_op_amadd_db_w;
            break;
        case 0xd5:
            op = la_op_amadd_db_d;
            break;
        case 0xd6:
            op = la_op_amand_db_w;
            break;
        case 0xd7:
            op = la_op_amand_db_d;
            break;
        case 0xd8:
            op = la_op_amor_db_w;
            break;
        case 0xd9:
            op = la_op_amor_db_d;
            break;
        case 0xda:
            op = la_op_amxor_db_w;
            break;
        case 0xdb:
            op = la_op_amxor_db_d;
            break;
        case 0xdc:
            op = la_op_ammax_db_w;
            break;
        case 0xdd:
            op = la_op_ammax_db_d;
            break;
        case 0xde:
            op = la_op_ammin_db_w;
            break;
        case 0xdf:
            op = la_op_ammin_db_d;
            break;
        case 0xe0:
            op = la_op_ammax_db_wu;
            break;
        case 0xe1:
            op = la_op_ammax_db_du;
            break;
        case 0xe2:
            op = la_op_ammin_db_wu;
            break;
        case 0xe3:
            op = la_op_ammin_db_du;
            break;
        case 0xe4:
            op = la_op_dbar;
            break;
        case 0xe5:
            op = la_op_ibar;
            break;
        case 0xe8:
            op = la_op_fldgt_s;
            break;
        case 0xe9:
            op = la_op_fldgt_d;
            break;
        case 0xea:
            op = la_op_fldle_s;
            break;
        case 0xeb:
            op = la_op_fldle_d;
            break;
        case 0xec:
            op = la_op_fstgt_s;
            break;
        case 0xed:
            op = la_op_fstgt_d;
            break;
        case 0xee:
            op = ls_op_fstle_s;
            break;
        case 0xef:
            op = la_op_fstle_d;
            break;
        case 0xf0:
            op = la_op_ldgt_b;
            break;
        case 0xf1:
            op = la_op_ldgt_h;
            break;
        case 0xf2:
            op = la_op_ldgt_w;
            break;
        case 0xf3:
            op = la_op_ldgt_d;
            break;
        case 0xf4:
            op = la_op_ldle_b;
            break;
        case 0xf5:
            op = la_op_ldle_h;
            break;
        case 0xf6:
            op = la_op_ldle_w;
            break;
        case 0xf7:
            op = la_op_ldle_d;
            break;
        case 0xf8:
            op = la_op_stgt_b;
            break;
        case 0xf9:
            op = la_op_stgt_h;
            break;
        case 0xfa:
            op = la_op_stgt_w;
            break;
        case 0xfb:
            op = la_op_stgt_d;
            break;
        case 0xfc:
            op = la_op_stle_b;
            break;
        case 0xfd:
            op = la_op_stle_h;
            break;
        case 0xfe:
            op = la_op_stle_w;
            break;
        case 0xff:
            op = la_op_stle_d;
            break;
        }
        break;
    case 0x10:
        op = la_op_beqz;
        break;
    case 0x11:
        op = la_op_bnez;
        break;
    case 0x12:
        switch ((insn >> 8) & 0x3) {
        case 0x0:
            op = la_op_bceqz;
            break;
        case 0x1:
            op = la_op_bcnez;
            break;
        }
        break;
    case 0x13:
        op = la_op_jirl;
        break;
    case 0x14:
        op = la_op_b;
        break;
    case 0x15:
        op = la_op_bl;
        break;
    case 0x16:
        op = la_op_beq;
        break;
    case 0x17:
        op = la_op_bne;
        break;
    case 0x18:
        op = la_op_blt;
        break;
    case 0x19:
        op = la_op_bge;
        break;
    case 0x1a:
        op = la_op_bltu;
        break;
    case 0x1b:
        op = la_op_bgeu;
        break;
    default:
        op = la_op_illegal;
        break;
    }
    dec->op = op;
}

/* operand extractors */
#define IM_5  5
#define IM_8  8
#define IM_12 12
#define IM_14 14
#define IM_15 15
#define IM_16 16
#define IM_20 20
#define IM_21 21
#define IM_26 26

static uint32_t operand_r1(uint32_t insn)
{
    return insn & 0x1f;
}

static uint32_t operand_r2(uint32_t insn)
{
    return (insn >> 5) & 0x1f;
}

static uint32_t operand_r3(uint32_t insn)
{
    return (insn >> 10) & 0x1f;
}

static uint32_t operand_r4(uint32_t insn)
{
    return (insn >> 15) & 0x1f;
}

static uint32_t operand_u6(uint32_t insn)
{
    return (insn >> 10) & 0x3f;
}

static uint32_t operand_bw1(uint32_t insn)
{
    return (insn >> 10) & 0x1f;
}

static uint32_t operand_bw2(uint32_t insn)
{
    return (insn >> 16) & 0x1f;
}

static uint32_t operand_bd1(uint32_t insn)
{
    return (insn >> 10) & 0x3f;
}

static uint32_t operand_bd2(uint32_t insn)
{
    return (insn >> 16) & 0x3f;
}

static uint32_t operand_sa2(uint32_t insn)
{
    return (insn >> 15) & 0x3;
}

static uint32_t operand_sa3(uint32_t insn)
{
    return (insn >> 15) & 0x3;
}

static int32_t operand_im20(uint32_t insn)
{
    int32_t imm = (int32_t)((insn >> 5) & 0xfffff);
    return imm > (1 << 19) ? imm - (1 << 20) : imm;
}

static int32_t operand_im16(uint32_t insn)
{
    int32_t imm = (int32_t)((insn >> 10) & 0xffff);
    return imm > (1 << 15) ? imm - (1 << 16) : imm;
}

static int32_t operand_im14(uint32_t insn)
{
    int32_t imm = (int32_t)((insn >> 10) & 0x3fff);
    return imm > (1 << 13) ? imm - (1 << 14) : imm;
}

static int32_t operand_im12(uint32_t insn)
{
    int32_t imm = (int32_t)((insn >> 10) & 0xfff);
    return imm > (1 << 11) ? imm - (1 << 12) : imm;
}

static int32_t operand_im8(uint32_t insn)
{
    int32_t imm = (int32_t)((insn >> 10) & 0xff);
    return imm > (1 << 7) ? imm - (1 << 8) : imm;
}

static uint32_t operand_sd(uint32_t insn)
{
    return insn & 0x3;
}

static uint32_t operand_sj(uint32_t insn)
{
    return (insn >> 5) & 0x3;
}

static uint32_t operand_cd(uint32_t insn)
{
    return insn & 0x7;
}

static uint32_t operand_cj(uint32_t insn)
{
    return (insn >> 5) & 0x7;
}

static uint32_t operand_code(uint32_t insn)
{
    return insn & 0x7fff;
}

static int32_t operand_whint(uint32_t insn)
{
    int32_t imm = (int32_t)(insn & 0x7fff);
    return imm > (1 << 14) ? imm - (1 << 15) : imm;
}

static int32_t operand_invop(uint32_t insn)
{
    int32_t imm = (int32_t)(insn & 0x1f);
    return imm > (1 << 4) ? imm - (1 << 5) : imm;
}

static int32_t operand_ofs21(uint32_t insn)
{
    int32_t imm = (((int32_t)insn & 0x1f) << 16) |
        ((insn >> 10) & 0xffff);
    return imm > (1 << 20) ? imm - (1 << 21) : imm;
}

static int32_t operand_ofs26(uint32_t insn)
{
    int32_t imm = (((int32_t)insn & 0x3ff) << 16) |
        ((insn >> 10) & 0xffff);
    return imm > (1 << 25) ? imm - (1 << 26) : imm;
}

static uint32_t operand_fcond(uint32_t insn)
{
    return (insn >> 15) & 0x1f;
}

static uint32_t operand_sel(uint32_t insn)
{
    return (insn >> 15) & 0x7;
}

/* decode operands */
static void decode_insn_operands(la_decode *dec)
{
    uint32_t insn = dec->insn;
    dec->codec = opcode_data[dec->op].codec;
    switch (dec->codec) {
    case la_codec_illegal:
    case la_codec_empty:
        break;
    case la_codec_2r:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        break;
    case la_codec_2r_u5:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_r3(insn);
        break;
    case la_codec_2r_u6:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_u6(insn);
        break;
    case la_codec_2r_2bw:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_bw1(insn);
        dec->r4 = operand_bw2(insn);
        break;
    case la_codec_2r_2bd:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_bd1(insn);
        dec->r4 = operand_bd2(insn);
        break;
    case la_codec_3r:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_r3(insn);
        break;
    case la_codec_3r_rd0:
        dec->r1 = 0;
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_r3(insn);
        break;
    case la_codec_3r_sa2:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_r3(insn);
        dec->r4 = operand_sa2(insn);
        break;
    case la_codec_3r_sa3:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_r3(insn);
        dec->r4 = operand_sa3(insn);
        break;
    case la_codec_4r:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_r3(insn);
        dec->r4 = operand_r4(insn);
        break;
    case la_codec_r_im20:
        dec->r1 = operand_r1(insn);
        dec->imm = operand_im20(insn);
        dec->bit = IM_20;
        break;
    case la_codec_2r_im16:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->imm = operand_im16(insn);
        dec->bit = IM_16;
        break;
    case la_codec_2r_im14:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->imm = operand_im14(insn);
        dec->bit = IM_14;
        break;
    case la_codec_r_im14:
        dec->r1 = operand_r1(insn);
        dec->imm = operand_im14(insn);
        dec->bit = IM_14;
        break;
    case la_codec_im5_r_im12:
        dec->imm2 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->imm = operand_im12(insn);
        dec->bit = IM_12;
        break;
    case la_codec_2r_im12:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->imm = operand_im12(insn);
        dec->bit = IM_12;
        break;
    case la_codec_2r_im8:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->imm = operand_im8(insn);
        dec->bit = IM_8;
        break;
    case la_codec_r_sd:
        dec->r1 = operand_sd(insn);
        dec->r2 = operand_r2(insn);
        break;
    case la_codec_r_sj:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_sj(insn);
        break;
    case la_codec_r_cd:
        dec->r1 = operand_cd(insn);
        dec->r2 = operand_r2(insn);
        break;
    case la_codec_r_cj:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_cj(insn);
        break;
    case la_codec_r_seq:
        dec->r1 = 0;
        dec->r2 = operand_r1(insn);
        dec->imm = operand_im8(insn);
        dec->bit = IM_8;
        break;
    case la_codec_code:
        dec->code = operand_code(insn);
        break;
    case la_codec_whint:
        dec->imm = operand_whint(insn);
        dec->bit = IM_15;
        break;
    case la_codec_invtlb:
        dec->imm = operand_invop(insn);
        dec->bit = IM_5;
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_r3(insn);
        break;
    case la_codec_r_ofs21:
        dec->imm = operand_ofs21(insn);
        dec->bit = IM_21;
        dec->r2 = operand_r2(insn);
        break;
    case la_codec_cj_ofs21:
        dec->imm = operand_ofs21(insn);
        dec->bit = IM_21;
        dec->r2 = operand_cj(insn);
        break;
    case la_codec_ofs26:
        dec->imm = operand_ofs26(insn);
        dec->bit = IM_26;
        break;
    case la_codec_cond:
        dec->r1 = operand_cd(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_r3(insn);
        dec->r4 = operand_fcond(insn);
        break;
    case la_codec_sel:
        dec->r1 = operand_r1(insn);
        dec->r2 = operand_r2(insn);
        dec->r3 = operand_r3(insn);
        dec->r4 = operand_sel(insn);
        break;
    }
}

/* format instruction */
static void append(char *s1, const char *s2, size_t n)
{
    size_t l1 = strlen(s1);
    if (n - l1 - 1 > 0) {
        strncat(s1, s2, n - l1);
    }
}

static void format_insn(char *buf, size_t buflen, size_t tab, la_decode *dec)
{
    char tmp[16];
    const char *fmt;

    fmt = opcode_data[dec->op].format;
    while (*fmt) {
        switch (*fmt) {
        case 'n': /* name */
            append(buf, opcode_data[dec->op].name, buflen);
            break;
        case 's':
            append(buf, "s", buflen);
            break;
        case 'd':
            append(buf, "d", buflen);
            break;
        case 'e': /* illegal */
            snprintf(tmp, sizeof(tmp), "%x", dec->insn);
            append(buf, tmp, buflen);
            break;
        case 't':
            while (strlen(buf) < tab) {
                append(buf, " ", buflen);
            }
            break;
        case '(':
            append(buf, "(", buflen);
            break;
        case ',':
            append(buf, ",", buflen);
            break;
        case '.':
            append(buf, ".", buflen);
            break;
        case ')':
            append(buf, ")", buflen);
            break;
        case '0': /* rd */
            append(buf, loongarch_r_normal_name[dec->r1], buflen);
            break;
        case '1': /* rj */
            append(buf, loongarch_r_normal_name[dec->r2], buflen);
            break;
        case '2': /* rk */
            append(buf, loongarch_r_normal_name[dec->r3], buflen);
            break;
        case '3': /* fd */
            append(buf, loongarch_f_normal_name[dec->r1], buflen);
            break;
        case '4': /* fj */
            append(buf, loongarch_f_normal_name[dec->r2], buflen);
            break;
        case '5': /* fk */
            append(buf, loongarch_f_normal_name[dec->r3], buflen);
            break;
        case '6': /* fa */
            append(buf, loongarch_f_normal_name[dec->r4], buflen);
            break;
        case 'A': /* sd */
            append(buf, loongarch_cr_normal_name[dec->r1], buflen);
            break;
        case 'B': /* sj */
            append(buf, loongarch_cr_normal_name[dec->r2], buflen);
            break;
        case 'C': /* r3 */
            snprintf(tmp, sizeof(tmp), "%x", dec->r3);
            append(buf, tmp, buflen);
            break;
        case 'D': /* r4 */
            snprintf(tmp, sizeof(tmp), "%x", dec->r4);
            append(buf, tmp, buflen);
            break;
        case 'E': /* r1 */
            snprintf(tmp, sizeof(tmp), "%x", dec->r1);
            append(buf, tmp, buflen);
            break;
        case 'F': /* fcsrd */
            append(buf, loongarch_r_normal_name[dec->r1], buflen);
            break;
        case 'G': /* fcsrs */
            append(buf, loongarch_r_normal_name[dec->r2], buflen);
            break;
        case 'H': /* cd */
            append(buf, loongarch_c_normal_name[dec->r1], buflen);
            break;
        case 'I': /* cj */
            append(buf, loongarch_c_normal_name[dec->r2], buflen);
            break;
        case 'J': /* code */
            snprintf(tmp, sizeof(tmp), "0x%x", dec->code);
            append(buf, tmp, buflen);
            break;
        case 'K': /* cond */
            switch (dec->r4) {
            case 0x0:
                append(buf, "caf", buflen);
                break;
            case 0x1:
                append(buf, "saf", buflen);
                break;
            case 0x2:
                append(buf, "clt", buflen);
                break;
            case 0x3:
                append(buf, "slt", buflen);
                break;
            case 0x4:
                append(buf, "ceq", buflen);
                break;
            case 0x5:
                append(buf, "seq", buflen);
                break;
            case 0x6:
                append(buf, "cle", buflen);
                break;
            case 0x7:
                append(buf, "sle", buflen);
                break;
            case 0x8:
                append(buf, "cun", buflen);
                break;
            case 0x9:
                append(buf, "sun", buflen);
                break;
            case 0xA:
                append(buf, "cult", buflen);
                break;
            case 0xB:
                append(buf, "sult", buflen);
                break;
            case 0xC:
                append(buf, "cueq", buflen);
                break;
            case 0xD:
                append(buf, "sueq", buflen);
                break;
            case 0xE:
                append(buf, "cule", buflen);
                break;
            case 0xF:
                append(buf, "sule", buflen);
                break;
            case 0x10:
                append(buf, "cne", buflen);
                break;
            case 0x11:
                append(buf, "sne", buflen);
                break;
            case 0x14:
                append(buf, "cor", buflen);
                break;
            case 0x15:
                append(buf, "sor", buflen);
                break;
            case 0x18:
                append(buf, "cune", buflen);
                break;
            case 0x19:
                append(buf, "sune", buflen);
                break;
            }
            break;
        case 'L': /* ca */
            append(buf, loongarch_c_normal_name[dec->r4], buflen);
            break;
        case 'M': /* cop */
            snprintf(tmp, sizeof(tmp), "0x%x", (dec->imm2) & 0x1f);
            append(buf, tmp, buflen);
            break;
        case 'i': /* sixx d */
            snprintf(tmp, sizeof(tmp), "%d", dec->imm);
            append(buf, tmp, buflen);
            break;
        case 'o': /* offset */
            snprintf(tmp, sizeof(tmp), "%d", (dec->imm) << 2);
            append(buf, tmp, buflen);
            break;
        case 'x': /* sixx x */
            switch (dec->bit) {
            case IM_5:
                snprintf(tmp, sizeof(tmp), "0x%x", (dec->imm) & 0x1f);
                append(buf, tmp, buflen);
                break;
            case IM_8:
                snprintf(tmp, sizeof(tmp), "0x%x", (dec->imm) & 0xff);
                append(buf, tmp, buflen);
                break;
            case IM_12:
                snprintf(tmp, sizeof(tmp), "0x%x", (dec->imm) & 0xfff);
                append(buf, tmp, buflen);
                break;
            case IM_14:
                snprintf(tmp, sizeof(tmp), "0x%x", (dec->imm) & 0x3fff);
                append(buf, tmp, buflen);
                break;
            case IM_15:
                snprintf(tmp, sizeof(tmp), "0x%x", (dec->imm) & 0x7fff);
                append(buf, tmp, buflen);
                break;
            case IM_16:
                snprintf(tmp, sizeof(tmp), "0x%x", (dec->imm) & 0xffff);
                append(buf, tmp, buflen);
                break;
            case IM_20:
                snprintf(tmp, sizeof(tmp), "0x%x", (dec->imm) & 0xfffff);
                append(buf, tmp, buflen);
                break;
            default:
                snprintf(tmp, sizeof(tmp), "0x%x", dec->imm);
                append(buf, tmp, buflen);
                break;
            }
            break;
        case 'X': /* offset x*/
            switch (dec->bit) {
            case IM_16:
                snprintf(tmp, sizeof(tmp), "0x%x",
                    ((dec->imm) << 2) & 0xffff);
                append(buf, tmp, buflen);
                break;
            case IM_21:
                snprintf(tmp, sizeof(tmp), "0x%x",
                    ((dec->imm) << 2) & 0x1fffff);
                append(buf, tmp, buflen);
                break;
            case IM_26:
                snprintf(tmp, sizeof(tmp), "0x%x",
                    ((dec->imm) << 2) & 0x3ffffff);
                append(buf, tmp, buflen);
                break;
            default:
                snprintf(tmp, sizeof(tmp), "0x%x", (dec->imm) << 2);
                append(buf, tmp, buflen);
                break;
            }
            break;
        case 'p': /* pc */
            snprintf(tmp, sizeof(tmp), "  # 0x%"PRIx32"",
                dec->pc + ((dec->imm) << 2));
            append(buf, tmp, buflen);
            break;
        default:
            break;
        }
        fmt++;
    }
}

/* disassemble instruction */
static void
disasm_insn(char *buf, size_t buflen, bfd_vma pc, unsigned long int insn)
{
    la_decode dec = { 0 };
    dec.pc = pc;
    dec.insn = insn;
    decode_insn_opcode(&dec);
    decode_insn_operands(&dec);
    format_insn(buf, buflen, 16, &dec);
}

int
print_insn_loongarch(bfd_vma memaddr, struct disassemble_info *info)
{
    char buf[128] = { 0 };
    bfd_byte buffer[INSNLEN];
    unsigned long insn;
    int status;

    status = (*info->read_memory_func)(memaddr, buffer, INSNLEN, info);
    if (status == 0) {
        insn = (uint32_t) bfd_getl32(buffer);
        (*info->fprintf_func)(info->stream, "%08" PRIx64 " ", insn);
    } else {
        (*info->memory_error_func)(status, memaddr, info);
        return -1;
    }
    disasm_insn(buf, sizeof(buf), memaddr, insn);
    (*info->fprintf_func)(info->stream, "\t%s", buf);
    return INSNLEN;
}
