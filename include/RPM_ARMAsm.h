/**
 * @file RPM_ARMAsm.h
 * @author Hello007
 * @brief ARM/Thumb assembler binary macros.
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_ARMASM_H
#define __RPM_ARMASM_H

#define THUMB_BL_HI(diff) (0b11110 << 11) | (((diff) >> 12) & 0x7FF)
#define THUMB_BL_LO(diff) (0b11111 << 11) | (((diff) >>  1) & 0x7FF)
#define THUMB_BLX_LO(diff) (0b11101 << 11) | (((diff) >> 1) & 0x7FF)
#define THUMB_B(diff) (0b11100 << 11) | ((diff) >> 1)
#define THUMB_BX(reg) (0b01000111 << 8) | ((((reg) >> 3) & 1) << 6) | (((reg) & 7) << 3)

#define THUMB_MOV_HI(dest, src) (0b01000110 << 8) | ((((dest) >> 3) & 1) << 7) | ((((src) >> 3) & 1) << 6) | (((src) & 7) << 3) | (((dest) & 7) << 0)

#define THUMB_LDR_PC_REL(reg, diff) (0b01001 << 11) | ((reg) << 8) | ((diff) >> 2)

#define THUMB_PUSH_LR 0xB500
#define THUMB_POP_PC 0xBD00
#define THUMB_PUSH_R4 0xB410
#define THUMB_POP_R4 0xBC10

#define ARM_BLX(jump) (0b1111101 << 25) | ((((jump) & 2) != 0) << 24) | (((jump) >> 2) & 0xFFFFFF)
#define ARM_BL(jump) (0b11101011 << 24)| (((jump) >> 2) & 0xFFFFFF)
#define ARM_B(jump) (0b11101010 << 24)| (((jump) >> 2) & 0xFFFFFF)

#endif