/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#pragma once

#include <boost/optional.hpp>

#include "frontend/A64/imm.h"
#include "frontend/A64/ir_emitter.h"
#include "frontend/A64/location_descriptor.h"
#include "frontend/A64/types.h"

namespace Dynarmic {
namespace A64 {

enum class AccType {
    NORMAL, VEC, STREAM, VECSTREAM, ATOMIC, ORDERED, UNPRIV, IFETCH, PTW, DC, IC, AT,
};

enum class MemOp {
    LOAD, STORE, PREFETCH,
};

struct TranslatorVisitor final {
    using instruction_return_type = bool;

    explicit TranslatorVisitor(IR::Block& block, LocationDescriptor descriptor) : ir(block, descriptor) {}

    A64::IREmitter ir;

    bool InterpretThisInstruction();
    bool UnpredictableInstruction();
    bool ReservedValue();
    bool UnallocatedEncoding();

    struct BitMasks {
        u64 wmask, tmask;
    };

    boost::optional<BitMasks> DecodeBitMasks(bool N, Imm<6> immr, Imm<6> imms, bool immediate);

    IR::U32U64 I(size_t bitsize, u64 value);
    IR::UAny X(size_t bitsize, Reg reg);
    void X(size_t bitsize, Reg reg, IR::U32U64 value);
    IR::U32U64 SP(size_t bitsize);
    void SP(size_t bitsize, IR::U32U64 value);

    IR::U128 V(size_t bitsize, Vec vec);
    void V(size_t bitsize, Vec vec, IR::U128 value);

    IR::UAny Mem(IR::U64 address, size_t size, AccType acctype);
    void Mem(IR::U64 address, size_t size, AccType acctype, IR::UAny value);

    IR::U32U64 SignExtend(IR::UAny value, size_t to_size);
    IR::U32U64 ZeroExtend(IR::UAny value, size_t to_size);
    IR::U32U64 ShiftReg(size_t bitsize, Reg reg, Imm<2> shift, IR::U8 amount);
    IR::U32U64 ExtendReg(size_t bitsize, Reg reg, Imm<3> option, u8 shift);

    // Data processing - Immediate - PC relative addressing
    bool ADR(Imm<2> immlo, Imm<19> immhi, Reg Rd);
    bool ADRP(Imm<2> immlo, Imm<19> immhi, Reg Rd);

    // Data processing - Immediate - Add/Sub
    bool ADD_imm(bool sf, Imm<2> shift, Imm<12> imm12, Reg Rn, Reg Rd);
    bool ADDS_imm(bool sf, Imm<2> shift, Imm<12> imm12, Reg Rn, Reg Rd);
    bool SUB_imm(bool sf, Imm<2> shift, Imm<12> imm12, Reg Rn, Reg Rd);
    bool SUBS_imm(bool sf, Imm<2> shift, Imm<12> imm12, Reg Rn, Reg Rd);

    // Data processing - Immediate - Logical
    bool AND_imm(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd);
    bool ORR_imm(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd);
    bool EOR_imm(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd);
    bool ANDS_imm(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd);

    // Data processing - Immediate - Move Wide
    bool MOVN(bool sf, Imm<2> hw, Imm<16> imm16, Reg Rd);
    bool MOVZ(bool sf, Imm<2> hw, Imm<16> imm16, Reg Rd);
    bool MOVK(bool sf, Imm<2> hw, Imm<16> imm16, Reg Rd);

    // Data processing - Immediate - Bitfield
    bool SBFM(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd);
    bool BFM(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd);
    bool UBFM(bool sf, bool N, Imm<6> immr, Imm<6> imms, Reg Rn, Reg Rd);

    // Data processing - Immediate - Extract
    bool EXTR(bool sf, bool N, Reg Rm, Imm<6> imms, Reg Rn, Reg Rd);

    // Conditional branch
    bool B_cond(Imm<19> imm19, Cond cond);

    // Exception generation
    bool SVC(Imm<16> imm16);
    bool HVC(Imm<16> imm16);
    bool SMC(Imm<16> imm16);
    bool BRK(Imm<16> imm16);
    bool HLT(Imm<16> imm16);
    bool DCPS1(Imm<16> imm16);
    bool DCPS2(Imm<16> imm16);
    bool DCPS3(Imm<16> imm16);

    // System
    bool MSR_imm(Imm<3> op1, Imm<4> CRm, Imm<3> op2);
    bool HINT(Imm<4> CRm, Imm<3> op2);
    bool NOP();
    bool YIELD();
    bool WFE();
    bool WFI();
    bool SEV();
    bool SEVL();
    bool XPAC_1(bool D, Reg Rd);
    bool XPAC_2();
    bool PACIA_1(bool Z, Reg Rn, Reg Rd);
    bool PACIA_2();
    bool PACIB_1(bool Z, Reg Rn, Reg Rd);
    bool PACIB_2();
    bool AUTIA_1(bool Z, Reg Rn, Reg Rd);
    bool AUTIA_2();
    bool AUTIB_1(bool Z, Reg Rn, Reg Rd);
    bool AUTIB_2();
    bool ESB();
    bool PSB();
    bool CLREX(Imm<4> CRm);
    bool DSB(Imm<4> CRm);
    bool DMB(Imm<4> CRm);
    bool ISB(Imm<4> CRm);
    bool SYS(Imm<3> op1, Imm<4> CRn, Imm<4> CRm, Imm<3> op2, Reg Rt);
    bool MSR_reg(bool o0, Imm<3> op1, Imm<4> CRn, Imm<4> CRm, Imm<3> op2, Reg Rt);
    bool SYSL(Imm<3> op1, Imm<4> CRn, Imm<4> CRm, Imm<3> op2, Reg Rt);
    bool MRS(bool o0, Imm<3> op1, Imm<4> CRn, Imm<4> CRm, Imm<3> op2, Reg Rt);

    // Unconditonal branch (Register)
    bool BR(Reg Rn);
    bool BRA(bool Z, bool M, Reg Rn, Reg Rm);
    bool BLR(Reg Rn);
    bool BLRA(bool Z, bool M, Reg Rn, Reg Rm);
    bool RET(Reg Rn);
    bool RETA(bool M);
    bool ERET();
    bool ERETA(bool M);
    bool DRPS();

    // Unconditonal branch (immediate)
    bool B_uncond(Imm<26> imm26);
    bool BL(Imm<26> imm26);

    // Compare and branch (immediate)
    bool CBZ(bool sf, Imm<19> imm19, Reg Rt);
    bool CBNZ(bool sf, Imm<19> imm19, Reg Rt);
    bool TBZ(Imm<1> b5, Imm<5> b40, Imm<14> imm14, Reg Rt);
    bool TBNZ(Imm<1> b5, Imm<5> b40, Imm<14> imm14, Reg Rt);

    // Loads and stores - Advanced SIMD Load/Store multiple structures
    bool ST4_mult_1(bool Q, Imm<2> size, Reg Rn, Reg Rt);
    bool ST4_mult_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Reg Rt);
    bool ST1_mult_1(bool Q, Imm<2> size, Reg Rn, Reg Rt);
    bool ST1_mult_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Reg Rt);
    bool ST3_mult_1(bool Q, Imm<2> size, Reg Rn, Reg Rt);
    bool ST3_mult_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Reg Rt);
    bool ST2_mult_1(bool Q, Imm<2> size, Reg Rn, Reg Rt);
    bool ST2_mult_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Reg Rt);
    bool LD4_mult_1(bool Q, Imm<2> size, Reg Rn, Reg Rt);
    bool LD4_mult_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Reg Rt);
    bool LD1_mult_1(bool Q, Imm<2> size, Reg Rn, Reg Rt);
    bool LD1_mult_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Reg Rt);
    bool LD3_mult_1(bool Q, Imm<2> size, Reg Rn, Reg Rt);
    bool LD3_mult_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Reg Rt);
    bool LD2_mult_1(bool Q, Imm<2> size, Reg Rn, Reg Rt);
    bool LD2_mult_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Reg Rt);

    // Loads and stores - Advanced SIMD Load/Store single structures
    bool ST1_sngl_1(bool Q, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool ST1_sngl_2(bool Q, Reg Rm, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool ST3_sngl_1(bool Q, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool ST3_sngl_2(bool Q, Reg Rm, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool ST2_sngl_1(bool Q, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool ST2_sngl_2(bool Q, Reg Rm, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool ST4_sngl_1(bool Q, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool ST4_sngl_2(bool Q, Reg Rm, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool LD1_sngl_1(bool Q, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool LD1_sngl_2(bool Q, Reg Rm, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool LD3_sngl_1(bool Q, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool LD3_sngl_2(bool Q, Reg Rm, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool LD1R_1(bool Q, Imm<2> size, Reg Rn, Vec Vt);
    bool LD1R_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Vec Vt);
    bool LD3R_1(bool Q, Imm<2> size, Reg Rn, Vec Vt);
    bool LD3R_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Vec Vt);
    bool LD2_sngl_1(bool Q, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool LD2_sngl_2(bool Q, Reg Rm, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool LD4_sngl_1(bool Q, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool LD4_sngl_2(bool Q, Reg Rm, bool S, Imm<2> size, Reg Rn, Vec Vt);
    bool LD2R_1(bool Q, Imm<2> size, Reg Rn, Vec Vt);
    bool LD2R_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Vec Vt);
    bool LD4R_1(bool Q, Imm<2> size, Reg Rn, Vec Vt);
    bool LD4R_2(bool Q, Reg Rm, Imm<2> size, Reg Rn, Vec Vt);

    // Loads and stores - Load/Store Exclusive
    bool STXRB(Reg Rs, Reg Rn, Reg Rt);
    bool STLXRB(Reg Rs, Reg Rn, Reg Rt);
    bool CASP(bool sz, bool L, Reg Rs, bool o0, Reg Rn, Reg Rt);
    bool LDXRB(Reg Rn, Reg Rt);
    bool LDAXRB(Reg Rn, Reg Rt);
    bool STLLRB(Reg Rn, Reg Rt);
    bool STLRB(Reg Rn, Reg Rt);
    bool CASB(bool L, Reg Rs, bool o0, Reg Rn, Reg Rt);
    bool LDLARB(Reg Rn, Reg Rt);
    bool LDARB(Reg Rn, Reg Rt);
    bool STXRH(Reg Rs, Reg Rn, Reg Rt);
    bool STLXRH(Reg Rs, Reg Rn, Reg Rt);
    bool LDXRH(Reg Rn, Reg Rt);
    bool LDAXRH(Reg Rn, Reg Rt);
    bool STLLRH(Reg Rn, Reg Rt);
    bool STLRH(Reg Rn, Reg Rt);
    bool CASH(bool L, Reg Rs, bool o0, Reg Rn, Reg Rt);
    bool LDLARH(Reg Rn, Reg Rt);
    bool LDARH(Reg Rn, Reg Rt);
    bool STXR(Reg Rs, Reg Rn, Reg Rt);
    bool STLXR(Reg Rs, Reg Rn, Reg Rt);
    bool STXP(bool sz, Reg Rs, Reg Rt2, Reg Rn, Reg Rt);
    bool STLXP(bool sz, Reg Rs, Reg Rt2, Reg Rn, Reg Rt);
    bool LDXR(Reg Rn, Reg Rt);
    bool LDAXR(Reg Rn, Reg Rt);
    bool LDXP(bool sz, Reg Rt2, Reg Rn, Reg Rt);
    bool LDAXP(bool sz, Reg Rt2, Reg Rn, Reg Rt);
    bool STLLR(Reg Rn, Reg Rt);
    bool STLR(Reg Rn, Reg Rt);
    bool CAS(bool L, Reg Rs, bool o0, Reg Rn, Reg Rt);
    bool LDLAR(Reg Rn, Reg Rt);
    bool LDAR(Reg Rn, Reg Rt);

    // Loads and stores - Load register (literal)
    bool LDR_lit_gen(bool opc_0, Imm<19> imm19, Reg Rt);
    bool LDR_lit_fpsimd(Imm<2> opc, Imm<19> imm19, Vec Vt);
    bool LDRSW_lit(Imm<19> imm19, Reg Rt);
    bool PRFM_lit(Imm<19> imm19, Imm<5> prfop);

    // Loads and stores - Load/Store no-allocate pair
    bool STNP_gen(Imm<7> imm7, Reg Rt2, Reg Rn, Reg Rt);
    bool LDNP_gen(Imm<7> imm7, Reg Rt2, Reg Rn, Reg Rt);
    bool STNP_fpsimd(Imm<2> opc, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt);
    bool LDNP_fpsimd(Imm<2> opc, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt);

    // Loads and stores - Load/Store register pair
    bool STP_LDP_gen(Imm<2> opc, bool not_postindex, bool wback, Imm<1> L, Imm<7> imm7, Reg Rt2, Reg Rn, Reg Rt);
    bool STP_fpsimd_1(Imm<2> opc, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt);
    bool STP_fpsimd_2(Imm<2> opc, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt);
    bool STP_fpsimd_3(Imm<2> opc, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt);
    bool LDP_fpsimd_1(Imm<2> opc, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt);
    bool LDP_fpsimd_2(Imm<2> opc, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt);
    bool LDP_fpsimd_3(Imm<2> opc, Imm<7> imm7, Vec Vt2, Reg Rn, Vec Vt);

    // Loads and stores - Load/Store register (immediate)
    bool load_store_register_immediate(bool wback, bool postindex, size_t scale, u64 offset, Imm<2> size, Imm<2> opc, Reg Rn, Reg Rt);
    bool STRx_LDRx_imm_1(Imm<2> size, Imm<2> opc, Imm<9> imm9, bool not_postindex, Reg Rn, Reg Rt);
    bool STRx_LDRx_imm_2(Imm<2> size, Imm<2> opc, Imm<12> imm12, Reg Rn, Reg Rt);
    bool STURx_LDURx(Imm<2> size, Imm<2> opc, Imm<9> imm9, Reg Rn, Reg Rt);
    bool PRFM_imm(Imm<12> imm12, Reg Rn, Reg Rt);
    bool STR_imm_fpsimd_1(Imm<2> size, Imm<9> imm9, bool not_postindex, Reg Rn, Vec Vt);
    bool STR_imm_fpsimd_2(Imm<2> size, Imm<12> imm12, Reg Rn, Vec Vt);
    bool LDR_imm_fpsimd_1(Imm<2> size, Imm<9> imm9, bool not_postindex, Reg Rn, Vec Vt);
    bool LDR_imm_fpsimd_2(Imm<2> size, Imm<12> imm12, Reg Rn, Vec Vt);
    bool STUR_fpsimd(Imm<2> size, Imm<9> imm9, Reg Rn, Vec Vt);
    bool LDUR_fpsimd(Imm<2> size, Imm<9> imm9, Reg Rn, Vec Vt);

    // Loads and stores - Load/Store register (unprivileged)
    bool STTRB(Imm<9> imm9, Reg Rn, Reg Rt);
    bool LDTRB(Imm<9> imm9, Reg Rn, Reg Rt);
    bool LDTRSB(Imm<2> opc, Imm<9> imm9, Reg Rn, Reg Rt);
    bool STTRH(Imm<9> imm9, Reg Rn, Reg Rt);
    bool LDTRH(Imm<9> imm9, Reg Rn, Reg Rt);
    bool LDTRSH(Imm<2> opc, Imm<9> imm9, Reg Rn, Reg Rt);
    bool STTR(Imm<2> size, Imm<9> imm9, Reg Rn, Reg Rt);
    bool LDTR(Imm<2> size, Imm<9> imm9, Reg Rn, Reg Rt);
    bool LDTRSW(Imm<9> imm9, Reg Rn, Reg Rt);

    // Loads and stores - Atomic memory options
    bool LDADDB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDCLRB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDEORB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDSETB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDSMAXB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDSMINB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDUMAXB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDUMINB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool SWPB(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDAPRB(Reg Rn, Reg Rt);
    bool LDADDH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDCLRH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDEORH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDSETH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDSMAXH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDSMINH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDUMAXH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDUMINH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool SWPH(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDAPRH(Reg Rn, Reg Rt);
    bool LDADD(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDCLR(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDEOR(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDSET(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDSMAX(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDSMIN(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDUMAX(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDUMIN(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool SWP(bool A, bool R, Reg Rs, Reg Rn, Reg Rt);
    bool LDAPR(Reg Rn, Reg Rt);

    // Loads and stores - Load/Store register (register offset)
    bool STRB_reg(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);
    bool LDRB_reg(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);
    bool LDRSB_reg(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);
    bool STR_reg_fpsimd(Imm<2> size, Reg Rm, Imm<3> option, bool S, Reg Rn, Vec Vt);
    bool LDR_reg_fpsimd(Imm<2> size, Reg Rm, Imm<3> option, bool S, Reg Rn, Vec Vt);
    bool STRH_reg(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);
    bool LDRH_reg(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);
    bool LDRSH_reg(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);
    bool STR_reg_gen(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);
    bool LDR_reg_gen(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);
    bool LDRSW_reg(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);
    bool PRFM_reg(Reg Rm, Imm<3> option, bool S, Reg Rn, Reg Rt);

    // Loads and stores - Load/Store register (pointer authentication)
    bool LDRA(bool M, bool S, Imm<9> imm9, bool W, Reg Rn, Reg Rt);

    // Data Processing - Register - 2 source
    bool UDIV(bool sf, Reg Rm, Reg Rn, Reg Rd);
    bool SDIV(bool sf, Reg Rm, Reg Rn, Reg Rd);
    bool LSLV(bool sf, Reg Rm, Reg Rn, Reg Rd);
    bool LSRV(bool sf, Reg Rm, Reg Rn, Reg Rd);
    bool ASRV(bool sf, Reg Rm, Reg Rn, Reg Rd);
    bool RORV(bool sf, Reg Rm, Reg Rn, Reg Rd);
    bool CRC32(bool sf, Reg Rm, Imm<2> sz, Reg Rn, Reg Rd);
    bool CRC32C(bool sf, Reg Rm, Imm<2> sz, Reg Rn, Reg Rd);
    bool PACGA(Reg Rm, Reg Rn, Reg Rd);

    // Data Processing - Register - 1 source
    bool RBIT_int(bool sf, Reg Rn, Reg Rd);
    bool REV16_int(bool sf, Reg Rn, Reg Rd);
    bool REV(bool sf, bool opc_0, Reg Rn, Reg Rd);
    bool CLZ_int(bool sf, Reg Rn, Reg Rd);
    bool CLS_int(bool sf, Reg Rn, Reg Rd);
    bool REV32_int(Reg Rn, Reg Rd);
    bool PACDA(bool Z, Reg Rn, Reg Rd);
    bool PACDB(bool Z, Reg Rn, Reg Rd);
    bool AUTDA(bool Z, Reg Rn, Reg Rd);
    bool AUTDB(bool Z, Reg Rn, Reg Rd);

    // Data Processing - Register - Logical (shifted register)
    bool AND_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool BIC_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool ORR_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool ORN_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool EOR_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool EON(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool ANDS_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool BICS(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);

    // Data Processing - Register - Add/Sub (shifted register)
    bool ADD_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool ADDS_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool SUB_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);
    bool SUBS_shift(bool sf, Imm<2> shift, Reg Rm, Imm<6> imm6, Reg Rn, Reg Rd);

    // Data Processing - Register - Add/Sub (shifted register)
    bool ADD_ext(bool sf, Reg Rm, Imm<3> option, Imm<3> imm3, Reg Rn, Reg Rd);
    bool ADDS_ext(bool sf, Reg Rm, Imm<3> option, Imm<3> imm3, Reg Rn, Reg Rd);
    bool SUB_ext(bool sf, Reg Rm, Imm<3> option, Imm<3> imm3, Reg Rn, Reg Rd);
    bool SUBS_ext(bool sf, Reg Rm, Imm<3> option, Imm<3> imm3, Reg Rn, Reg Rd);

    // Data Processing - Register - Add/Sub (with carry)
    bool ADC(bool sf, Reg Rm, Reg Rn, Reg Rd);
    bool ADCS(bool sf, Reg Rm, Reg Rn, Reg Rd);
    bool SBC(bool sf, Reg Rm, Reg Rn, Reg Rd);
    bool SBCS(bool sf, Reg Rm, Reg Rn, Reg Rd);

    // Data Processing - Register - Conditional compare
    bool CCMN_reg(bool sf, Reg Rm, Cond cond, Reg Rn, Imm<4> nzcv);
    bool CCMP_reg(bool sf, Reg Rm, Cond cond, Reg Rn, Imm<4> nzcv);
    bool CCMN_imm(bool sf, Imm<5> imm5, Cond cond, Reg Rn, Imm<4> nzcv);
    bool CCMP_imm(bool sf, Imm<5> imm5, Cond cond, Reg Rn, Imm<4> nzcv);

    // Data Processing - Register - Conditional select
    bool CSEL(bool sf, Reg Rm, Cond cond, Reg Rn, Reg Rd);
    bool CSINC(bool sf, Reg Rm, Cond cond, Reg Rn, Reg Rd);
    bool CSINV(bool sf, Reg Rm, Cond cond, Reg Rn, Reg Rd);
    bool CSNEG(bool sf, Reg Rm, Cond cond, Reg Rn, Reg Rd);

    // Data Processing - Register - 3 source
    bool MADD(bool sf, Reg Rm, Reg Ra, Reg Rn, Reg Rd);
    bool MSUB(bool sf, Reg Rm, Reg Ra, Reg Rn, Reg Rd);
    bool SMADDL(Reg Rm, Reg Ra, Reg Rn, Reg Rd);
    bool SMSUBL(Reg Rm, Reg Ra, Reg Rn, Reg Rd);
    bool SMULH(Reg Rm, Reg Rn, Reg Rd);
    bool UMADDL(Reg Rm, Reg Ra, Reg Rn, Reg Rd);
    bool UMSUBL(Reg Rm, Reg Ra, Reg Rn, Reg Rd);
    bool UMULH(Reg Rm, Reg Rn, Reg Rd);

    // Data Processing - FP and SIMD - AES
    bool AESE(Vec Vn, Vec Vd);
    bool AESD(Vec Vn, Vec Vd);
    bool AESMC(Vec Vn, Vec Vd);
    bool AESIMC(Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SHA
    bool SHA1C(Vec Vm, Vec Vn, Vec Vd);
    bool SHA1P(Vec Vm, Vec Vn, Vec Vd);
    bool SHA1M(Vec Vm, Vec Vn, Vec Vd);
    bool SHA1SU0(Vec Vm, Vec Vn, Vec Vd);
    bool SHA256H(Vec Vm, Vec Vn, Vec Vd);
    bool SHA256H2(Vec Vm, Vec Vn, Vec Vd);
    bool SHA256SU1(Vec Vm, Vec Vn, Vec Vd);
    bool SHA1H(Vec Vn, Vec Vd);
    bool SHA1SU1(Vec Vn, Vec Vd);
    bool SHA256SU0(Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Scalar copy
    bool DUP_elt_1(Imm<5> imm5, Vec Vn, Vec Vd);
    bool DUP_elt_2(bool Q, Imm<5> imm5, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Scalar three
    bool FMULX_vec_1(Vec Vm, Vec Vn, Vec Vd);
    bool FMULX_vec_2(bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMULX_vec_3(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMULX_vec_4(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FCMEQ_reg_1(Vec Vm, Vec Vn, Vec Vd);
    bool FCMEQ_reg_2(bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FCMEQ_reg_3(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FCMEQ_reg_4(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FRECPS_1(Vec Vm, Vec Vn, Vec Vd);
    bool FRECPS_2(bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FRECPS_3(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FRECPS_4(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FRSQRTS_1(Vec Vm, Vec Vn, Vec Vd);
    bool FRSQRTS_2(bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FRSQRTS_3(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FRSQRTS_4(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FCMGE_reg_1(Vec Vm, Vec Vn, Vec Vd);
    bool FCMGE_reg_2(bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FCMGE_reg_3(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FCMGE_reg_4(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FACGE_1(Vec Vm, Vec Vn, Vec Vd);
    bool FACGE_2(bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FACGE_3(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FACGE_4(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FABD_1(Vec Vm, Vec Vn, Vec Vd);
    bool FABD_2(bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FABD_3(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FABD_4(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FCMGT_reg_1(Vec Vm, Vec Vn, Vec Vd);
    bool FCMGT_reg_2(bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FCMGT_reg_3(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FCMGT_reg_4(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FACGT_1(Vec Vm, Vec Vn, Vec Vd);
    bool FACGT_2(bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FACGT_3(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FACGT_4(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Two register misc
    bool FCVTNS_1(Vec Vn, Vec Vd);
    bool FCVTNS_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTNS_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTNS_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCVTMS_1(Vec Vn, Vec Vd);
    bool FCVTMS_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTMS_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTMS_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCVTAS_1(Vec Vn, Vec Vd);
    bool FCVTAS_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTAS_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTAS_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool SCVTF_int_1(Vec Vn, Vec Vd);
    bool SCVTF_int_2(bool sz, Vec Vn, Vec Vd);
    bool SCVTF_int_3(bool Q, Vec Vn, Vec Vd);
    bool SCVTF_int_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCMGT_zero_1(Vec Vn, Vec Vd);
    bool FCMGT_zero_2(bool sz, Vec Vn, Vec Vd);
    bool FCMGT_zero_3(bool Q, Vec Vn, Vec Vd);
    bool FCMGT_zero_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCMEQ_zero_1(Vec Vn, Vec Vd);
    bool FCMEQ_zero_2(bool sz, Vec Vn, Vec Vd);
    bool FCMEQ_zero_3(bool Q, Vec Vn, Vec Vd);
    bool FCMEQ_zero_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCMLT_1(Vec Vn, Vec Vd);
    bool FCMLT_2(bool sz, Vec Vn, Vec Vd);
    bool FCMLT_3(bool Q, Vec Vn, Vec Vd);
    bool FCMLT_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCVTPS_1(Vec Vn, Vec Vd);
    bool FCVTPS_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTPS_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTPS_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCVTZS_int_1(Vec Vn, Vec Vd);
    bool FCVTZS_int_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTZS_int_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTZS_int_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FRECPE_1(Vec Vn, Vec Vd);
    bool FRECPE_2(bool sz, Vec Vn, Vec Vd);
    bool FRECPE_3(bool Q, Vec Vn, Vec Vd);
    bool FRECPE_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FRECPX_1(Vec Vn, Vec Vd);
    bool FRECPX_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTNU_1(Vec Vn, Vec Vd);
    bool FCVTNU_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTNU_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTNU_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCVTMU_1(Vec Vn, Vec Vd);
    bool FCVTMU_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTMU_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTMU_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCVTAU_1(Vec Vn, Vec Vd);
    bool FCVTAU_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTAU_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTAU_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool UCVTF_int_1(Vec Vn, Vec Vd);
    bool UCVTF_int_2(bool sz, Vec Vn, Vec Vd);
    bool UCVTF_int_3(bool Q, Vec Vn, Vec Vd);
    bool UCVTF_int_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCMGE_zero_1(Vec Vn, Vec Vd);
    bool FCMGE_zero_2(bool sz, Vec Vn, Vec Vd);
    bool FCMGE_zero_3(bool Q, Vec Vn, Vec Vd);
    bool FCMGE_zero_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCMLE_1(Vec Vn, Vec Vd);
    bool FCMLE_2(bool sz, Vec Vn, Vec Vd);
    bool FCMLE_3(bool Q, Vec Vn, Vec Vd);
    bool FCMLE_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCVTPU_1(Vec Vn, Vec Vd);
    bool FCVTPU_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTPU_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTPU_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FCVTZU_int_1(Vec Vn, Vec Vd);
    bool FCVTZU_int_2(bool sz, Vec Vn, Vec Vd);
    bool FCVTZU_int_3(bool Q, Vec Vn, Vec Vd);
    bool FCVTZU_int_4(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FRSQRTE_1(Vec Vn, Vec Vd);
    bool FRSQRTE_2(bool sz, Vec Vn, Vec Vd);
    bool FRSQRTE_3(bool Q, Vec Vn, Vec Vd);
    bool FRSQRTE_4(bool Q, bool sz, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Scalar three same extra
    bool SQRDMLAH_vec_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQRDMLAH_vec_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQRDMLSH_vec_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQRDMLSH_vec_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Scalar two-register misc
    bool SUQADD_1(Imm<2> size, Vec Vn, Vec Vd);
    bool SUQADD_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool SQABS_1(Imm<2> size, Vec Vn, Vec Vd);
    bool SQABS_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool CMGT_zero_1(Imm<2> size, Vec Vn, Vec Vd);
    bool CMGT_zero_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool CMEQ_zero_1(Imm<2> size, Vec Vn, Vec Vd);
    bool CMEQ_zero_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool CMLT_1(Imm<2> size, Vec Vn, Vec Vd);
    bool CMLT_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool ABS_1(Imm<2> size, Vec Vn, Vec Vd);
    bool ABS_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool SQXTN_1(Imm<2> size, Vec Vn, Reg Rd);
    bool SQXTN_2(bool Q, Imm<2> size, Vec Vn, Reg Rd);
    bool USQADD_1(Imm<2> size, Vec Vn, Vec Vd);
    bool USQADD_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool SQNEG_1(Imm<2> size, Vec Vn, Vec Vd);
    bool SQNEG_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool CMGE_zero_1(Imm<2> size, Vec Vn, Vec Vd);
    bool CMGE_zero_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool CMLE_1(Imm<2> size, Vec Vn, Vec Vd);
    bool CMLE_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool NEG_1(Imm<2> size, Vec Vn, Vec Vd);
    bool NEG_2(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool SQXTUN_1(Imm<2> size, Vec Vn, Reg Rd);
    bool SQXTUN_2(bool Q, Imm<2> size, Vec Vn, Reg Rd);
    bool UQXTN_1(Imm<2> size, Vec Vn, Reg Rd);
    bool UQXTN_2(bool Q, Imm<2> size, Vec Vn, Reg Rd);
    bool FCVTXN_1(bool sz, Vec Vn, Reg Rd);
    bool FCVTXN_2(bool Q, bool sz, Vec Vn, Reg Rd);

    // Data Processing - FP and SIMD - SIMD Scalar pairwise
    bool ADDP_pair(Imm<2> size, Vec Vn, Vec Vd);
    bool FMAXNMP_pair_1(Vec Vn, Vec Vd);
    bool FMAXNMP_pair_2(bool sz, Vec Vn, Vec Vd);
    bool FADDP_pair_1(Vec Vn, Vec Vd);
    bool FADDP_pair_2(bool sz, Vec Vn, Vec Vd);
    bool FMAXP_pair_1(Vec Vn, Vec Vd);
    bool FMAXP_pair_2(bool sz, Vec Vn, Vec Vd);
    bool FMINNMP_pair_1(Vec Vn, Vec Vd);
    bool FMINNMP_pair_2(bool sz, Vec Vn, Vec Vd);
    bool FMINP_pair_1(Vec Vn, Vec Vd);
    bool FMINP_pair_2(bool sz, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Scalar three different
    bool SQDMLAL_vec_1(Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SQDMLAL_vec_2(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SQDMLSL_vec_1(Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SQDMLSL_vec_2(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SQDMULL_vec_1(Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SQDMULL_vec_2(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Scalar three same
    bool SQADD_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQADD_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQSUB_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQSUB_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMGT_reg_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMGT_reg_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMGE_reg_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMGE_reg_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SSHL_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SSHL_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQSHL_reg_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQSHL_reg_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SRSHL_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SRSHL_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQRSHL_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQRSHL_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool ADD_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool ADD_vector(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMTST_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMTST_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQDMULH_vec_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQDMULH_vec_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UQADD_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UQADD_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UQSUB_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UQSUB_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMHI_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMHI_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMHS_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMHS_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool USHL_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool USHL_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UQSHL_reg_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UQSHL_reg_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool URSHL_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool URSHL_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UQRSHL_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UQRSHL_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SUB_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SUB_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMEQ_reg_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool CMEQ_reg_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQRDMULH_vec_1(Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SQRDMULH_vec_2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Scalar shift by immediate
    bool SSHR_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SSHR_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SSRA_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SSRA_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SRSHR_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SRSHR_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SRSRA_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SRSRA_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SHL_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SHL_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SQSHL_imm_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SQSHL_imm_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SQSHRN_1(Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool SQSHRN_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool SQRSHRN_1(Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool SQRSHRN_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool SCVTF_fix_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SCVTF_fix_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool FCVTZS_fix_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool FCVTZS_fix_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool USHR_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool USHR_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool USRA_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool USRA_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool URSHR_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool URSHR_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool URSRA_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool URSRA_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SRI_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SRI_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SLI_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SLI_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SQSHLU_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SQSHLU_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool UQSHL_imm_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool UQSHL_imm_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool SQSHRUN_1(Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool SQSHRUN_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool SQRSHRUN_1(Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool SQRSHRUN_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool UQSHRN_1(Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool UQSHRN_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool UQRSHRN_1(Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool UQRSHRN_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool UCVTF_fix_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool UCVTF_fix_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool FCVTZU_fix_1(Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);
    bool FCVTZU_fix_2(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Scalar x indexed element
    bool SQDMLAL_elt_1(Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool SQDMLAL_elt_2(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool SQDMLSL_elt_1(Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool SQDMLSL_elt_2(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool SQDMULL_elt_1(Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool SQDMULL_elt_2(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool SQDMULH_elt_1(Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool SQDMULH_elt_2(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool SQRDMULH_elt_1(Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool SQRDMULH_elt_2(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMLA_elt_1(bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMLA_elt_2(bool sz, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMLA_elt_3(bool Q, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMLA_elt_4(bool Q, bool sz, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMLS_elt_1(bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMLS_elt_2(bool sz, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMLS_elt_3(bool Q, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMLS_elt_4(bool Q, bool sz, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMUL_elt_1(bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMUL_elt_2(bool sz, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMUL_elt_3(bool Q, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMUL_elt_4(bool Q, bool sz, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool SQRDMLAH_elt_1(Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool SQRDMLAH_elt_2(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool SQRDMLSH_elt_1(Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool SQRDMLSH_elt_2(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMULX_elt_1(bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMULX_elt_2(bool sz, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMULX_elt_3(bool Q, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMULX_elt_4(bool Q, bool sz, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Table Lookup
    bool TBL(bool Q, Vec Vm, Imm<2> len, Vec Vn, Vec Vd);
    bool TBX(bool Q, Vec Vm, Imm<2> len, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Permute
    bool UZP1(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool TRN1(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool ZIP1(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UZP2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool TRN2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool ZIP2(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Extract
    bool EXT(bool Q, Vec Vm, Imm<4> imm4, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Copy
    bool DUP_gen(bool Q, Imm<5> imm5, Reg Rn, Vec Vd);
    bool SMOV(bool Q, Imm<5> imm5, Vec Vn, Reg Rd);
    bool UMOV(bool Q, Imm<5> imm5, Vec Vn, Reg Rd);
    bool INS_gen(Imm<5> imm5, Reg Rn, Vec Vd);
    bool INS_elt(Imm<5> imm5, Imm<4> imm4, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Three same
    bool FMAXNM_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMAXNM_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMLA_vec_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMLA_vec_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FADD_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FADD_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMAX_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMAX_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMINNM_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMINNM_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMLS_vec_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMLS_vec_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FSUB_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FSUB_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMIN_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMIN_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMAXNMP_vec_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMAXNMP_vec_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FADDP_vec_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FADDP_vec_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMUL_vec_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMUL_vec_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMAXP_vec_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMAXP_vec_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FDIV_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FDIV_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMINNMP_vec_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMINNMP_vec_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);
    bool FMINP_vec_1(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMINP_vec_2(bool Q, bool sz, Vec Vm, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Two-register misc
    bool FRINTN_1(bool Q, Vec Vn, Vec Vd);
    bool FRINTN_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FRINTM_1(bool Q, Vec Vn, Vec Vd);
    bool FRINTM_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FABS_1(bool Q, Vec Vn, Vec Vd);
    bool FABS_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FRINTP_1(bool Q, Vec Vn, Vec Vd);
    bool FRINTP_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FRINTZ_1(bool Q, Vec Vn, Vec Vd);
    bool FRINTZ_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FRINTA_1(bool Q, Vec Vn, Vec Vd);
    bool FRINTA_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FRINTX_1(bool Q, Vec Vn, Vec Vd);
    bool FRINTX_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FNEG_1(bool Q, Vec Vn, Vec Vd);
    bool FNEG_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FRINTI_1(bool Q, Vec Vn, Vec Vd);
    bool FRINTI_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FSQRT_1(bool Q, Vec Vn, Vec Vd);
    bool FSQRT_2(bool Q, bool sz, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Three same extra
    bool SDOT_vec(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UDOT_vec(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool FCMLA_vec(bool Q, Imm<2> size, Vec Vm, Imm<2> rot, Vec Vn, Vec Vd);
    bool FCADD_vec(bool Q, Imm<2> size, Vec Vm, Imm<1> rot, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD Two register misc
    bool REV64_asimd(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool REV16_asimd(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool SADDLP(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool CLS_asimd(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool CNT(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool SADALP(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool XTN(bool Q, Imm<2> size, Vec Vn, Reg Rd);
    bool FCVTN(bool Q, bool sz, Vec Vn, Reg Rd);
    bool FCVTL(bool Q, bool sz, Reg Rn, Vec Vd);
    bool URECPE(bool Q, bool sz, Vec Vn, Vec Vd);
    bool REV32_asimd(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool UADDLP(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool CLZ_asimd(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool UADALP(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool SHLL(bool Q, Imm<2> size, Reg Rn, Vec Vd);
    bool NOT(bool Q, Vec Vn, Vec Vd);
    bool RBIT_asimd(bool Q, Vec Vn, Vec Vd);
    bool URSQRTE(bool Q, bool sz, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD across lanes
    bool SADDLV(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool SMAXV(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool SMINV(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool ADDV(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool FMAXNMV_1(bool Q, Vec Vn, Vec Vd);
    bool FMAXNMV_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FMAXV_1(bool Q, Vec Vn, Vec Vd);
    bool FMAXV_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FMINNMV_1(bool Q, Vec Vn, Vec Vd);
    bool FMINNMV_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool FMINV_1(bool Q, Vec Vn, Vec Vd);
    bool FMINV_2(bool Q, bool sz, Vec Vn, Vec Vd);
    bool UADDLV(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool UMAXV(bool Q, Imm<2> size, Vec Vn, Vec Vd);
    bool UMINV(bool Q, Imm<2> size, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD three different
    bool SADDL(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SADDW(bool Q, Imm<2> size, Reg Rm, Vec Vn, Vec Vd);
    bool SSUBL(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SSUBW(bool Q, Imm<2> size, Reg Rm, Vec Vn, Vec Vd);
    bool ADDHN(bool Q, Imm<2> size, Vec Vm, Vec Vn, Reg Rd);
    bool SABAL(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SUBHN(bool Q, Imm<2> size, Vec Vm, Vec Vn, Reg Rd);
    bool SABDL(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SMLAL_vec(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SMLSL_vec(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool SMULL_vec(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool PMULL(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool UADDL(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool UADDW(bool Q, Imm<2> size, Reg Rm, Vec Vn, Vec Vd);
    bool USUBL(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool USUBW(bool Q, Imm<2> size, Reg Rm, Vec Vn, Vec Vd);
    bool RADDHN(bool Q, Imm<2> size, Vec Vm, Vec Vn, Reg Rd);
    bool UABAL(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool RSUBHN(bool Q, Imm<2> size, Vec Vm, Vec Vn, Reg Rd);
    bool UABDL(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool UMLAL_vec(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool UMLSL_vec(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);
    bool UMULL_vec(bool Q, Imm<2> size, Reg Rm, Reg Rn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD three same
    bool SHADD(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SRHADD(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SHSUB(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SMAX(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SMIN(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SABD(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SABA(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool MLA_vec(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool MUL_vec(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SMAXP(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool SMINP(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool ADDP_vec(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool FMLAL_vec_1(bool Q, bool sz, Reg Rm, Reg Rn, Vec Vd);
    bool FMLAL_vec_2(bool Q, bool sz, Reg Rm, Reg Rn, Vec Vd);
    bool AND_asimd(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool BIC_asimd_reg(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool FMLSL_vec_1(bool Q, bool sz, Reg Rm, Reg Rn, Vec Vd);
    bool FMLSL_vec_2(bool Q, bool sz, Reg Rm, Reg Rn, Vec Vd);
    bool ORR_asimd_reg(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool ORN_asimd(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool UHADD(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool URHADD(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UHSUB(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UMAX(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UMIN(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UABD(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UABA(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool MLS_vec(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool PMUL(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UMAXP(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool UMINP(bool Q, Imm<2> size, Vec Vm, Vec Vn, Vec Vd);
    bool EOR_asimd(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool BSL(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool BIT(bool Q, Vec Vm, Vec Vn, Vec Vd);
    bool BIF(bool Q, Vec Vm, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD modified immediate
    bool MOVI(bool Q, bool op, Imm<1> a, Imm<1> b, Imm<1> c, Imm<4> cmode, Imm<1> d, Imm<1> e, Imm<1> f, Imm<1> g, Imm<1> h, Reg Rd);
    bool ORR_asimd_imm(bool Q, Imm<1> a, Imm<1> b, Imm<1> c, Imm<1> d, Imm<1> e, Imm<1> f, Imm<1> g, Imm<1> h, Reg Rd);
    bool FMOV_1(bool Q, Imm<1> a, Imm<1> b, Imm<1> c, Imm<1> d, Imm<1> e, Imm<1> f, Imm<1> g, Imm<1> h, Reg Rd);
    bool FMOV_2(bool Q, bool op, Imm<1> a, Imm<1> b, Imm<1> c, Imm<1> d, Imm<1> e, Imm<1> f, Imm<1> g, Imm<1> h, Reg Rd);
    bool MVNI(bool Q, Imm<1> a, Imm<1> b, Imm<1> c, Imm<4> cmode, Imm<1> d, Imm<1> e, Imm<1> f, Imm<1> g, Imm<1> h, Reg Rd);
    bool BIC_imm(bool Q, Imm<1> a, Imm<1> b, Imm<1> c, Imm<1> d, Imm<1> e, Imm<1> f, Imm<1> g, Imm<1> h, Reg Rd);

    // Data Processing - FP and SIMD - SIMD Shfit by immediate
    bool SHRN(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool RSHRN(bool Q, Imm<4> immh, Imm<3> immb, Vec Vn, Reg Rd);
    bool SSHLL(bool Q, Imm<4> immh, Imm<3> immb, Reg Rn, Vec Vd);
    bool USHLL(bool Q, Imm<4> immh, Imm<3> immb, Reg Rn, Vec Vd);

    // Data Processing - FP and SIMD - SIMD x indexed element
    bool SMLAL_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool SMLSL_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool MUL_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool SMULL_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool SDOT_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FMLAL_elt_1(bool Q, bool sz, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool FMLAL_elt_2(bool Q, bool sz, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool FMLSL_elt_1(bool Q, bool sz, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool FMLSL_elt_2(bool Q, bool sz, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool MLA_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool UMLAL_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool MLS_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool UMLSL_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool UMULL_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Reg Rn, Vec Vd);
    bool UDOT_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, bool H, Vec Vn, Vec Vd);
    bool FCMLA_elt(bool Q, Imm<2> size, bool L, bool M, Vec Vm, Imm<2> rot, bool H, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Cryptographic three register
    bool SM3TT1A(Vec Vm, Imm<2> imm2, Vec Vn, Vec Vd);
    bool SM3TT1B(Vec Vm, Imm<2> imm2, Vec Vn, Vec Vd);
    bool SM3TT2A(Vec Vm, Imm<2> imm2, Vec Vn, Vec Vd);
    bool SM3TT2B(Vec Vm, Imm<2> imm2, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SHA512 three register
    bool SHA512H(Vec Vm, Vec Vn, Vec Vd);
    bool SHA512H2(Vec Vm, Vec Vn, Vec Vd);
    bool SHA512SU1(Vec Vm, Vec Vn, Vec Vd);
    bool RAX1(Vec Vm, Vec Vn, Vec Vd);
    bool SM3PARTW1(Vec Vm, Vec Vn, Vec Vd);
    bool SM3PARTW2(Vec Vm, Vec Vn, Vec Vd);
    bool SM4EKEY(Vec Vm, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Cryptographic four register
    bool EOR3(Vec Vm, Vec Va, Vec Vn, Vec Vd);
    bool BCAX(Vec Vm, Vec Va, Vec Vn, Vec Vd);
    bool SM3SS1(Vec Vm, Vec Va, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - SHA512 two register
    bool SHA512SU0(Vec Vn, Vec Vd);
    bool SM4E(Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Conversion between floating point and fixed point
    bool SCVTF_float_fix(bool sf, Imm<2> type, Imm<6> scale, Reg Rn, Vec Vd);
    bool UCVTF_float_fix(bool sf, Imm<2> type, Imm<6> scale, Reg Rn, Vec Vd);
    bool FCVTZS_float_fix(bool sf, Imm<2> type, Imm<6> scale, Vec Vn, Reg Rd);
    bool FCVTZU_float_fix(bool sf, Imm<2> type, Imm<6> scale, Vec Vn, Reg Rd);

    // Data Processing - FP and SIMD - Conversion between floating point and integer
    bool FCVTNS_float(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool FCVTNU_float(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool SCVTF_float_int(bool sf, Imm<2> type, Reg Rn, Vec Vd);
    bool UCVTF_float_int(bool sf, Imm<2> type, Reg Rn, Vec Vd);
    bool FCVTAS_float(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool FCVTAU_float(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool FMOV_float_gen(bool sf, Imm<2> type, Vec Vn, Vec Vd);
    bool FCVTPS_float(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool FCVTPU_float(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool FCVTMS_float(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool FCVTMU_float(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool FCVTZS_float_int(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool FCVTZU_float_int(bool sf, Imm<2> type, Vec Vn, Reg Rd);
    bool FJCVTZS(Vec Vn, Reg Rd);

    // Data Processing - FP and SIMD - Floating point data processing
    bool FMOV_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FABS_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FNEG_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FSQRT_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FCVT_float(Imm<2> type, Imm<2> opc, Vec Vn, Vec Vd);
    bool FRINTN_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FRINTP_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FRINTM_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FRINTZ_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FRINTA_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FRINTX_float(Imm<2> type, Vec Vn, Vec Vd);
    bool FRINTI_float(Imm<2> type, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Floating point compare
    bool FCMP_float(Imm<2> type, Vec Vm, Vec Vn);
    bool FCMPE_float(Imm<2> type, Vec Vm, Vec Vn);

    // Data Processing - FP and SIMD - Floating point immediate
    bool FMOV_float_imm(Imm<2> type, Imm<8> imm8, Vec Vd);

    // Data Processing - FP and SIMD - Floating point conditional compare
    bool FCCMP_float(Imm<2> type, Vec Vm, Cond cond, Vec Vn, Imm<4> nzcv);
    bool FCCMPE_float(Imm<2> type, Vec Vm, Cond cond, Vec Vn, Imm<4> nzcv);

    // Data Processing - FP and SIMD - Floating point data processing two register
    bool FMUL_float(Imm<2> type, Vec Vm, Vec Vn, Vec Vd);
    bool FDIV_float(Imm<2> type, Vec Vm, Vec Vn, Vec Vd);
    bool FADD_float(Imm<2> type, Vec Vm, Vec Vn, Vec Vd);
    bool FSUB_float(Imm<2> type, Vec Vm, Vec Vn, Vec Vd);
    bool FMAX_float(Imm<2> type, Vec Vm, Vec Vn, Vec Vd);
    bool FMIN_float(Imm<2> type, Vec Vm, Vec Vn, Vec Vd);
    bool FMAXNM_float(Imm<2> type, Vec Vm, Vec Vn, Vec Vd);
    bool FMINNM_float(Imm<2> type, Vec Vm, Vec Vn, Vec Vd);
    bool FNMUL_float(Imm<2> type, Vec Vm, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Floating point conditional select
    bool FCSEL_float(Imm<2> type, Vec Vm, Cond cond, Vec Vn, Vec Vd);

    // Data Processing - FP and SIMD - Floating point data processing three register
    bool FMADD_float(Imm<2> type, Vec Vm, Vec Va, Vec Vn, Vec Vd);
    bool FMSUB_float(Imm<2> type, Vec Vm, Vec Va, Vec Vn, Vec Vd);
    bool FNMADD_float(Imm<2> type, Vec Vm, Vec Va, Vec Vn, Vec Vd);
    bool FNMSUB_float(Imm<2> type, Vec Vm, Vec Va, Vec Vn, Vec Vd);
};

} // namespace A64
} // namespace Dynarmic
