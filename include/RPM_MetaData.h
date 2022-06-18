/**
 * @file RPM_MetaData.h
 * @author Hello007
 * @brief RPM Metadata structures and control.
 * @version 0.1
 * @date 2022-01-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __RPM_METADATA_H
#define __RPM_METADATA_H

namespace rpm {
    class MetaData;
}

#include "RPM_Types.h"
#include "RPM_Control.h"
#include "RPM_Module.h"

namespace rpm {
    enum MetaValueType : u8 {
        /**
         * @brief C-string metavalue inside the RPM string table.
         */
        STRING,
        /**
         * @brief 32-bit integer metavalue.
         */
        INT
    };

    struct MetaValue {
        /**
         * @brief Lookup name key of the metavalue.
         */
        RPM_NAMEOFS     Name;
        /**
         * @brief Type of the value's content.
         */
        MetaValueType   Type;
        u8              Reserved;
        union {
            /**
             * @brief Value for MetaValueType::STRING.
             */
            RPM_NAMEOFS StringValue;
            /**
             * @brief Value for MetaValueType::INT.
             */
            int         IntValue;
        };
    };

    struct MetaData {
        /**
         * @brief Number of metavalues in this value set.
         */
        u32         ValueCount;
        /**
         * @brief Inline array of metavalues.
         */
        MetaValue   Values[];

        /**
         * @brief Finds a metavalue by name using string comparison.
         * 
         * @param module Parent module of this metadata section for name resolution.
         * @param name Name of the searched value.
         * @return Value with a matching name key, or null if none found.
         */
        MetaValue* FindValue(Module* module, const char* name);

        /**
         * @brief Shortcut to get a value of an INT metavalue.
         * 
         * @param module Parent module of this metadata section for name resolution.
         * @param name Name of the searched value.
         * @param defaultValue Value to return if none was matched.
         * @return Content of the metavalue with a matching name key, or 'defaultValue' if none found. 
         */
        int GetInt(Module* module, const char* name, int defaultValue);

        /**
         * @brief Shortcut to get a value of a STRING metavalue.
         * 
         * @param module Parent module of this metadata section for name resolution.
         * @param name Name of the searched value.
         * @param defaultValue Value to return if none was matched.
         * @return Content of the metavalue with a matching name key, or 'defaultValue' if none found. 
         */
        const char* GetString(Module* module, const char* name, const char* defaultValue);

        /**
         * @brief Gets an INT metavalue's content by name, returning 0 if none was matched.
         * 
         * @param module Parent module of this metadata section for name resolution.
         * @param name Name of the searched value.
         * @return Content of the metavalue with a matching name key, or 0 if none found.  
         */
        INLINE int GetInt(Module* module, const char* name) {
            return GetInt(module, name, 0);
        }

        /**
         * @brief Gets a STRING metavalue's content by name, returning null if none was matched.
         * 
         * @param module Parent module of this metadata section for name resolution.
         * @param name Name of the searched value.
         * @return Content of the metavalue with a matching name key, or null if none found.  
         */
        INLINE const char* GetString(Module* module, const char* name) {
            return GetString(module, name, nullptr);
        }
    };
}

#endif