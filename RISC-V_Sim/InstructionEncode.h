#pragma once

#include <cstdint>

uint32_t Create_lb();
uint32_t Create_lh();
uint32_t Create_lw();
uint32_t Create_ld();
uint32_t Create_lbu();
uint32_t Create_lhu();
uint32_t Create_lwu();
uint32_t Create_fence();
uint32_t Create_fence_i();
uint32_t Create_addi();
uint32_t Create_slli();
uint32_t Create_slti();
uint32_t Create_sltiu();
uint32_t Create_xori();
uint32_t Create_srli();
uint32_t Create_srai();
uint32_t Create_ori();
uint32_t Create_andi();
uint32_t Create_auipc();
uint32_t Create_addiw();
uint32_t Create_slliw();
uint32_t Create_srliw();
uint32_t Create_sraiw();
uint32_t Create_sb();
uint32_t Create_sh();
uint32_t Create_sw();
uint32_t Create_sd();
uint32_t Create_add();
uint32_t Create_sub();
uint32_t Create_sll();
uint32_t Create_slt();
uint32_t Create_sltu();
uint32_t Create_xor();
uint32_t Create_srl();
uint32_t Create_sra();
uint32_t Create_or();
uint32_t Create_and();
uint32_t Create_lui();
uint32_t Create_addw();
uint32_t Create_subw();
uint32_t Create_sllw();
uint32_t Create_srlw();
uint32_t Create_sraw();
uint32_t Create_beq();
uint32_t Create_bne();
uint32_t Create_blt();
uint32_t Create_bge();
uint32_t Create_bltu();
uint32_t Create_bgeu();
uint32_t Create_jalr();
uint32_t Create_jal();
uint32_t Create_ecall();
uint32_t Create_ebreak();
uint32_t Create_csrrw();
uint32_t Create_csrrs();
uint32_t Create_csrrc();
uint32_t Create_csrrwi();
uint32_t Create_csrrsi();
uint32_t Create_csrrci();