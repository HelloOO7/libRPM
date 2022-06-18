/**
 * @file RPM_Control.h
 * @author Hello007
 * @brief Structures and enums of RPM code metadata (control sections).
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __RPM_CONTROL_H
#define __RPM_CONTROL_H

#include "RPM_Types.h"
#include "RPM_EnumFlagOperators.h"

namespace rpm {
	#define MODULE_BASE "base"

	enum SymbolType : u8 {
		/**
		 * @brief The type of this symbol is undefined.
		 */
		RPM_SYMTYPE_NULL,
		/**
		 * @brief This symbol is a generic value.
		 */
		RPM_SYMTYPE_VALUE,
		/**
		 * @brief This symbol is a block of ARM-coded instructions.
		 */
		RPM_SYMTYPE_FUNCTION_ARM,
		/**
		 * @brief This symbol is a block if Thumb-coded instructions.
		 */
		RPM_SYMTYPE_FUNCTION_THM,
		/**
		 * @brief This symbol denotes a section start.
		 */
		RPM_SYMTYPE_SECTION
	};

	enum SymbolAttr : u8 {
		/**
		 * @brief This symbol can be imported from foreign modules.
		 */
		RPM_SYMATTR_EXPORT = 1 << 0,
		/**
		 * @brief This symbol needs to be resolved from a dependency module.
		 */
		RPM_SYMATTR_IMPORT = 1 << 1
	};

	DEFINE_ENUM_FLAG_OPERATORS(SymbolAttr)

	/**
	 * @brief A tag for the procedure used for code relocation.
	 * 
	 * The input values for a relocation request shall be:
	 * 
	 * S = The address which is being relocated
	 * T = The address of the symbol to which the relocation should point
	 * L = Size of the target symbol
	 * D = T - S
	 * PD = D adjusted by ARM prefetch
	 * X = 1 if the instruction sets used at S and T are different.
	 * 
	 * w = Word
	 * lh = Low Halfword
	 * hh = High Halfword
	 */
	enum RelTargetType : u8 {
		/**
		 * @brief R_ARM_ABS32 | *Sw = T
		 */
		RPM_REL_TGTTYPE_OFFSET,
		/**
		 * @brief R_ARM_THM_PC22 | *Slh = THUMB_BL_HI(PD); *Shh = (X ? THUMB_BLX_LO : THUMB_BL_LO)(PD)
		 */
		RPM_REL_TGTTYPE_THUMB_BL,
		/**
		 * @brief R_ARM_PC24 | *Sw = (X ? ARM_BLX : ARM_BL)(PD)
		 */
		RPM_REL_TGTTYPE_ARM_BL,
		/**
		 * @brief Proprietary unconditional branch routine for Thumb.
		 * 
		 * If abs(PD) < 2048, THUMB_B(PD) is used.
		 * Otherwise, the following code is generated:
		 * 
		 * PUSH {LR}
		 * [RPM_REL_TGTTYPE_THUMB_BL]
		 * POP {PC}
		 */
		RPM_REL_TGTTYPE_THUMB_B,
		/**
		 * @brief R_ARM_PC24 | *Sw = ARM_B(PD)
		 */
		RPM_REL_TGTTYPE_ARM_B,
		/**
		 * @brief This procedure copies the entire target memory block to the source address.
		 * 
		 * memcpy(T, S, L)
		 */
		RPM_REL_TGTTYPE_FULL_COPY,
		/**
		 * @brief Proprietary unconditional branch routine for Thumb with workarounds for subroutines with large parameter counts.
		 * 
		 * If abs(PD) < 2048, THUMB_B(PD) is used.
		 * 
		 * Otherwise, ARM registers will be used as branch target storage. This can take up to 16 bytes of code, which shouldn't be a problem with such large subroutines.
		 * The exact implementation is as follows:
		 * 
		 * == Version 0.9 onward ==
		 * 
		 * PUSH R4 		@Backup a low register so that we can LDR into it
		 * LDR R4, =OFFSET
		 * MOV R12, 4 	@Move the LDRd value to the ARM switch register
		 * POP R4	 	@Restore the low register value
		 * BX R12		@Branch to the high register
		 * .word OFFSET T
		 * 
		 * == Before version 0.9 ==
		 * 
		 * MOV R11, R4 	@Backup a low register so that we can LDR into it
		 * LDR R4, =OFFSET
		 * MOV R12, 4 	@Move the LDRd value to the high register
		 * MOV R4, R11 	@Restore the low register value
		 * BX R12		@Branch to the high register
		 * .word OFFSET T
		 */
		RPM_REL_TGTTYPE_THUMB_B_SAFESTACK
	};

	/**
	 * @brief Offset of an RPM name. This is a 16-bit index into the STR0 string table.
	 */
	typedef u16 RPM_NAMEOFS;
	/**
	 * @brief Type representing a 32-bit FNV1a RPM name hash.
	 * 
	 * See rpm::Util::HashName for exact algorithm.
	 */
	typedef u32 RPM_NAMEHASH;

	/**
	 * @brief Symbol reference that points to a memory address.
	 */
	struct InternalAddress {
		u32  Value:31;
		u32  Local:1;
	};

	/**
	 * @brief Reference to an RPM symbol's location.
	 */
	struct Address {
		union {
			/**
			 * @brief Address of either an in-module location or a global absolute symbol.
			 */
			InternalAddress Internal;
			/**
			 * @brief Hash referencing an external imported symbol's internal address.
			 */
			RPM_NAMEHASH	ImportHash;
		};
	};

	/**
	 * @brief An RPM-flavoured code metadata symbol.
	 */
	struct Symbol {
		RPM_NAMEOFS	Name;
		u16			Size;
		Address     Addr;
		SymbolType  Type;
		SymbolAttr  Attr;
		u16			Reserved;
	};

	/**
	 * @brief Description of a relocation target's location.
	 */
	RPM_PACK(struct RelTarget {
		/**
		 * @brief Offset of the relocation target.
		 * 
		 * If the target is within the module, it is relative to the code segment. Otherwise, its exact value is defined by the rpm::mgr::ExternalRelocator implementation.
		 */
		u32   			Offset;
		/**
		 * @brief -1 for internal relocation, otherwise, this indexes to the name of an abstract module hosting the relocation target.
		 */
		u8				ExternModuleIndex;
		/**
		 * @brief Procedure to use for the relocation.
		 */
		RelTargetType 	RelProcType;
	});

	/**
	 * @brief Source of the relocation address that represents a symbol within the same module.
	 */
	struct RelSource {
		/**
		 * @brief Index of the symbol within the RPM.
		 */
		u16 SymbNo;
	};

	/**
	 * @brief An RPM-flavoured relocation information entry.
	 */
	struct Relocation {
		/**
		 * @brief A location which the relocation should be written to.
		 */
		RelTarget     Target;
		/**
		 * @brief A location towards which the relocation should point.
		 */
		RelSource	  Source;
	};

	static_assert(sizeof(Relocation) == 0x8);

	/**
	 * @brief A list of RPM names of external modules.
	 */
	struct ModuleNameList {
		u16 		Count;
		RPM_NAMEOFS Entries[];
	};

	/**
	 * @brief A list of RPM relocation entries.
	 */
	struct RelocationList {
		u32			Count;
		Relocation 	Relocations[];
	};
}

#endif