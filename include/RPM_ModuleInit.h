/**
 * @file RPM_ModuleInit.h
 * @author Hello007
 * @brief Types for module pre-load stage.
 * @version 0.1
 * @date 2022-01-17
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_MODULEINIT_H
#define __RPM_MODULEINIT_H

namespace rpm{
	namespace init {
		/**
		 * @brief Intermediate RPM module allocation work type.
		 */
		typedef void* ModuleAllocation;
	}
}

#endif