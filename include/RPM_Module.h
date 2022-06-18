/**
 * @file RPM_Module.h
 * @author Hello007
 * @brief Direct RPM binary executable access.
 * @version 0.1
 * @date 2022-01-17
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_MODULE_H
#define __RPM_MODULE_H

namespace rpm {
	class Module;
}

#include "RPM_Types.h"
#include "RPM_DllExport.h"
#include "RPM_Control.h"
#include "RPM_Util.h"
#include "RPM_ModuleInit.h"
#include "RPM_ModuleFixLevel.h"
#include "RPM_MetaData.h"
#include "RPM_CpuUtil.h"
#include "RPM_DllApi.h"

namespace rpm {
	/**
	 * @brief The Dynamically Linked Executable File structure. 
	 * This class can be directly assigned from a pointer to RPM module data.
	 */
	class Module {
	public:
		struct SymbolSection {
			#define SYM0_MAGIC MAGIC('S', 'Y', 'M', '0')

			u32 			Magic;

			ModuleNameList* ExternModules;

			u16				FirstExportSymbolIdx;
			u16				ExportSymbolCount;
			u16				FirstImportSymbolIdx;
			u16				ImportSymbolCount;
			RPM_NAMEHASH*	ExportSymbolHashTable;

			u32 			SymbolCount;
			Symbol  		Symbols[];
		};

		struct RelocationSection {
			#define REL0_MAGIC MAGIC('R', 'E', 'L', '0')

			u32 			Magic;

			u32 			BaseAddress;

			RelocationList* InternalRelocations;
			RelocationList* InternalImportRelocations;
			RelocationList* ExternalRelocations;
			ModuleNameList* ExternModules;
		};

		struct StringSection {
			#define STR0_MAGIC MAGIC('S', 'T', 'R', '0')

			u32		Magic;
			char	Strings[];
		};

		struct MetaDataSection {
			#define META_MAGIC MAGIC('M', 'E', 'T', 'A')

			u32		  Magic;
			MetaData  MetaValues;
		};

		struct InfoSection {
			#define INFO_MAGIC MAGIC('I', 'N', 'F', 'O')

			u32					Magic;
			SymbolSection*		Symbols;
			RelocationSection*	Relocations;
			StringSection*		Strings;
			u8*					Code;
			u32					CodeSize;
			MetaDataSection*	MetaValueSection;
			u32					Reserved1;
		};

		struct DllExec {
			#define DLLEXEC_MAGIC MAGIC('D', 'L', 'X', 'H')

			u32 		 Magic;
			u32 		 Version;
			InfoSection* Info;
			u32			 Reserved1;
		};

		/**
		 * @brief Creates a module from an intermediate allocation.
		 * 
		 * @param alloc The allocated and loaded module data.		 
		 */
		RPM_PUBLIC static Module* InitModule(rpm::init::ModuleAllocation alloc);

		/**
		 * @brief Calculates the byte-size of a module after fixing.
		 * 
		 * @param fixLevel Desired fixing level.
		 * @return size_t The unaligned size of the fixed module.
		 */
		size_t CalcFixedSize(rpm::FixLevel fixLevel);

		/**
		 * @brief Interprets this module as an unassociated data type pointer.
		 * 
		 * @return void* to this.
		 */
		INLINE void* GetRawData() {
			return static_cast<void*>(this);
		}

		/**
		 * @brief Gets the unaligned size of this module in memory.
		 * 
		 * Fixing the module will affect this value.
		 */
		INLINE size_t GetModuleSize() {
			return m_Size;
		}

		/**
		 * @brief Internal method to set the fixed module size.
		 * 
		 * @param newSize The size of the module in bytes.
		 */
		INLINE void UpdateModuleSizeAfterFixing(size_t newSize) {
			m_Size = newSize;
		}

		/**
		 * @brief Resolves a RPM name offset to a C string.
		 * 
		 * @param offs RPM_NAMEOFS representing the RPM name.
		 * @return char* to the name.
		 */
		RPM_PUBLIC const char* GetString(RPM_NAMEOFS offs);

		/**
		 * @brief Get the memory address of the code segment.
		 * 
		 * @return The code segment's memory address as a char*.
		 */
		INLINE u8* GetCode() {
			return (u8*) m_Exec->Info->Code;
		}

		/**
		 * @brief Get the size of the code segment.
		 * 
		 * @return Size of the code segment in bytes.
		 */
		INLINE u32 GetCodeSize() {
			return m_Exec->Info->CodeSize;
		}

		/**
		 * @brief Get the module's Symbol table section (.symtab).
		 */
		INLINE SymbolSection* GetSymbols() {
			return m_Exec->Info->Symbols;
		}

		/**
		 * @brief Get the module's Relocation table section (.rel).
		 */
		INLINE RelocationSection* GetRelocations() {
			return m_Exec->Info->Relocations;
		}

		/**
		 * @brief Get the pointer to the meta data storage from within the module's metadata section.
		 * 
		 * @return Pointer to a MetaData, or null if the module does not have a metadata section.
		 */
		INLINE MetaData* GetMetaData() {
			if (m_Exec->Info->MetaValueSection) {
				return &m_Exec->Info->MetaValueSection->MetaValues;
			}
			return nullptr;
		}

		/**
		 * @brief Gets the previous loaded module in the runtime linked list.
		 * 
		 * @return m_PrevModule 
		 */
		INLINE Module* GetPrevModule() {
			return m_PrevModule;
		}

		/**
		 * @brief Sets the previous loaded module in the runtime linked list.
		 */
		INLINE void SetPrevModule(Module* m) {
			m_PrevModule = m;
		}

		/**
		 * @brief Gets the next loaded module in the runtime linked list.
		 * 
		 * @return m_NextModule 
		 */
		INLINE Module* GetNextModule() {
			return m_NextModule;
		}

		/**
		 * @brief Sets the next loaded module in the runtime linked list.
		 */
		INLINE void SetNextModule(Module* m) {
			m_NextModule = m;
		}

		/**
		 * @brief Removes all control section references from the module's info header.
		 */
		INLINE void DisableControl() {
			InfoSection* info = m_Exec->Info;
			info->Symbols = nullptr;
			info->Relocations = nullptr;
			info->Strings = nullptr;
			info->MetaValueSection = nullptr;
		}

		/**
		 * @brief Resolves mutual import/export symbols between two modules.
		 * 
		 * @param other The friend module to import/export from/to.
		 */
		void LinkWithModule(Module* other);

		/**
		 * @brief Resolves import symbols from another module.
		 * 
		 * @param other The module to import symbols from.
		 */
		void ImportModule(Module* other);

		/**
		 * @brief Looks up a symbol index by name using string comparison.
		 * 
		 * @param name Name of the searched symbol.
		 * @return Index of the symbol, or 0xFFFF if none was found.
		 */
		RPM_PUBLIC u16 FindSymbolIdx(const char* name);

		/**
		 * @brief Looks up an exported symbol index by name using hashtables.
		 * 
		 * @param name Name of the searched exported symbol.
		 * @return Index of the exported symbol, or 0xFFFF if none was found.
		 */
		RPM_PUBLIC u16 FindExportSymbolIdx(const char* name);

		/**
		 * @brief Looks up a symbol by name using string comparison.
		 * 
		 * @param name Name of the searched symbol.
		 * @return Symbol with name 'name', or null if none was found.
		 */
		RPM_PUBLIC Symbol* FindSymbol(const char* name);

		/**
		 * @brief Looks up an exported symbol by name using hashtables.
		 * 
		 * @param name Name of the searched exported symbol.
		 * @return Exported symbol with name 'name', or null if none was found.
		 */
		RPM_PUBLIC Symbol* FindExportSymbol(const char* name);

		/**
		 * @brief Safely retrieves a symbol from the module's symbol section.
		 * 
		 * @param index Index of the symbol within the module.
		 * @return Pointer to the symbol at index 'index', or null if the index is invalid or if there is no symbol section.
		 */
		RPM_PUBLIC Symbol* GetSymbol(u16 index);

		/**
		 * @brief Gets the physical memory address of a symbol in code within this module.
		 * 
		 * Shortcut to Util::GetSymbolAddressAbsolute(module, symbol)
		 * 
		 * @param sym The symbol to get the address of.
		 * @return Address of the code symbol as a char*.
		 */
		RPM_PUBLIC u8* GetSymbolAddressAbsolute(Symbol* sym);

		/**
		 * @brief Gets the number of unique named modules that this module has external symbols within.
		 * 
		 * @return u16 Number of external symbol modules. 
		 */
		RPM_PUBLIC u16 GetSymExternModuleCount();

		/**
		 * @brief Gets the name of an unique named module that this module has external symbols within.
		 * 
		 * @return u16 Index of the external module within the data table.
		 */
		RPM_PUBLIC const char* GetSymExternModuleName(u16 index);

		/**
		 * @brief Gets the number of unique named modules that this module relocates externally.
		 * 
		 * @return u16 Number of external symbol modules. 
		 */
		RPM_PUBLIC u16 GetRelExternModuleCount();

		/**
		 * @brief Gets the name of an unique named module that this module relocates externally.
		 * 
		 * @return u16 Index of the external module within the data table.
		 */
		RPM_PUBLIC const char* GetRelExternModuleName(u16 index);

		/**
		 * @brief Relocates all control sections of this module.
		 * 
		 * A ReserveFlag is set to prevent this from being done more than once.
		 */
		void RelocateControl();

		/**
		 * @brief Performs all relocations that point to a given imported symbol.
		 * 
		 * @param symIndex Index of the imported symbol in this module's symbol table.
		 */
		void RelocateByImportSymbol(u32 symIndex);

		/**
		 * @brief Performs all local internal relocations.
		 */
		void RelocateInternal();

		/**
		 * @brief Relocates this module's file offset to a memory pointer.
		 * 
		 * @param pptr Pointer to the location of the memory pointer.
		 */
		void RelocFilePtr(void* pptr);

		/**
		 * @brief Relocates this module's file offset to a memory pointer, handling -1 and 0 as invalid null pointers.
		 * 
		 * @param pptr Pointer to the location of the memory pointer.
		 */
		void RelocFilePtrNonNull(void* pptr);

		/**
		 * @brief Checks this module's version and magic constants against current libRPM implementation.
		 * 
		 * @return true If all constants have been matched.
		 * @return false If at least one of the constants has an irregular value.
		 */
		bool Verify();

	private:
		#define RPM_MAGIC MAGIC('R', 'P', 'M', '0')

		u32			m_Magic;
		u32			m_Size;
		DllExec* 	m_Exec;
		u32			m_ReserveFlags;
		
		Module*		m_PrevModule;
		Module*     m_NextModule;

		enum ReserveFlag {
			RPM_RSVFLAG_CONTROL_RELOCATED = 0x1,
			RPM_RSVFLAG_CODE_RELOCATED_INTERNAL = 0x2,
		};

		bool GetReserveFlag(ReserveFlag flag) {
			return m_ReserveFlags & flag;
		}

		void SetReserveFlag(ReserveFlag flag) {
			m_ReserveFlags |= flag;
		}
	};
}

#endif