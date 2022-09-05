#ifndef __RPM_METADATA_CPP
#define __RPM_METADATA_CPP

#include "RPM_Module.h"
#include "RPM_MetaData.h"
#include "Util/exl_StrEq.h"

namespace rpm {
    MetaValue* MetaData::FindValue(Module* module, const char* name) {
        RPM_ASSERT(module);
        MetaValue* val = Values;
        if (val) {
            for (u32 i = 0; i < ValueCount; i++, val++) {
                const char* valueStr = module->GetString(val->Name);
                if (valueStr) {
                    if (strequal(valueStr, name)) {
                        return val;
                    }
                }
            }
        }
        return nullptr;
    }

    int MetaData::GetInt(Module* module, const char* name, int defaultValue) {
        MetaValue* val = FindValue(module, name);
        if (val && val->Type == MetaValueType::INT) {
            return val->IntValue;
        }
        return defaultValue;
    }

    const char* MetaData::GetString(Module* module, const char* name, const char* defaultValue) {
        MetaValue* val = FindValue(module, name);
        if (val && val->Type == MetaValueType::STRING) {
            return module->GetString(val->StringValue);
        }
        return defaultValue;
    }
}

#endif