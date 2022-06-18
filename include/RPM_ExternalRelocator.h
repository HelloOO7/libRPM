/**
 * @file RPM_ExternalRelocator.h
 * @author Hello007
 * @brief Interface for processing external relocations.
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_EXTERNALRELOCATOR_H
#define __RPM_EXTERNALRELOCATOR_H

#include "RPM_Types.h"
#include "RPM_Module.h"
#include "RPM_Control.h"

namespace rpm {
	namespace mgr {
		/**
		 * @brief Interface for processing external relocations.
		 */
		class ExternalRelocator {
			public:
				/**
				 * @brief Virtual function to handle a relocation not contained in the source module.
				 * 
				 * @param module The module that the relocation points from.
				 * @param rel Relocation to process.
				 */
				virtual void ProcessRelocation(rpm::Module* module, rpm::Relocation* rel) {};
		};
	}
}

#endif