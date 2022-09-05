#ifndef __RPM_MODULELISTENER_H
#define __RPM_MODULELISTENER_H

#include "RPM_Module.h"
#include "RPM_ModuleManager.h"

namespace rpm {
    namespace mgr {
        /**
         * @brief Module event type.
         */
        enum ModuleEvent {
			LOADED,
			UNLOADED,
			READY,
			STARTED,
			LINKED,
			FIXED
        };

		/**
		 * @brief Interface for reacting to module events.
		 */
		class ModuleListener {
			private:
				ModuleListener* m_Next;

				friend class ModuleManager;

			public:
				/**
				 * @brief Virtual function to handle a relocation not contained in the source module.
				 * 
				 * @param module The module that the relocation points from.
				 * @param rel Relocation to process.
				 */
				virtual void OnEvent(rpm::mgr::ModuleManager* mgr, rpm::Module* module, ModuleEvent event) {};
		};
	}
}

#endif