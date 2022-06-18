/**
 * @file RPM_ModuleFixLevel.h
 * @author Hello007
 * @brief Enum declarations for module fixing.
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 */
#ifndef __RPM_MODULEFIXLEVEL_H
#define __RPM_MODULEFIXLEVEL_H

namespace rpm{
	/**
	 * @brief Level of section-stripping of a runtime module.
	 * 
	 * The "fixing" process is borrowed from the CRO documentation at https://gist.github.com/wwylele/325d53ee6a0f1dff6aa3473377335d93.
	 * Programmers should choose wisely when deciding which fix level to use as the library does not have precautions for prematurely stripped sections.
	 */
	enum FixLevel {
		/**
		 * @brief The module should be left in memory untouched.
		 */
		NONE,
		/**
		 * @brief The internal relocation table should be stripped.
		 */
		INTERNAL_RELOCATIONS,
		/**
		 * @brief All control data should be stripped.
		 */
		ALL_NONCODE
	};
}

#endif