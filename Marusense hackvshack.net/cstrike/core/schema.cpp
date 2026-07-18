#include "schema.h"

// used: [stl] vector
#include <vector>
// used: [stl] find_if
#include <algorithm>

// used: getworkingpath
#include "../core.h"

// used: ischemasystem
#include "interfaces.h"
#include "../sdk/interfaces/ischemasystem.h"

// used: l_print
#include "../utilities/log.h"

bool SCHEMA::FindDeclaredClass(CSchemaTypeScope* typeScope, const char* className, CSchemaDeclaredClass** outClass)
{
    if (!typeScope || !className || !outClass)
        return false;

    for (uint16_t i = 0; i < typeScope->NumDeclaredClasses; ++i)
    {
        CSchemaDeclaredClass* declaredClass = typeScope->mDeclaredClasses[i].mDeclaredClass;
        if (declaredClass && declaredClass->Name && strcmp(declaredClass->Name, className) == 0)
        {
            *outClass = declaredClass;
            return true;
        }
    }

    return false;
}

std::uint32_t SCHEMA::Get(const char* moduleName, const char* className, const char* fieldName)
{
    uint32_t res = 0;

    if (!I::SchemaSystem)
    {
        L_PRINT(LOG_INFO) << CS_XOR("SchemaSystem is null");
        return res;
    }

    CSchemaTypeScope* typeScope = I::SchemaSystem->FindTypeScopeForModule(moduleName);
    if (!typeScope)
    {
        L_PRINT(LOG_INFO) << CS_XOR("TypeScope not found for module: ") << moduleName;
        return res;
    }

    CSchemaDeclaredClass* declaredClass = nullptr;
    if (!SCHEMA::FindDeclaredClass(typeScope, className, &declaredClass) || !declaredClass)
    {
        L_PRINT(LOG_INFO) << CS_XOR("DeclaredClass not found: ") << className;
        return res;
    }

    CSchemaClassInfo* schemaClass = declaredClass->mClass;
    if (!schemaClass)
    {
        L_PRINT(LOG_INFO) << CS_XOR("SchemaClass is null: ") << className;
        return res;
    }

    if (!schemaClass->Fields)
    {
        L_PRINT(LOG_INFO) << CS_XOR("Fields is null in class: ") << className;
        return res;
    }

    for (uint16_t i = 0; i < schemaClass->NumFields; ++i)
    {
        CSchemaClassField& field = schemaClass->Fields[i];

        if (field.Name && strcmp(field.Name, fieldName) == 0)
        {
            L_PRINT(LOG_INFO) << CS_XOR("Schema field found: ") << fieldName
                << CS_XOR(" offset: ") << field.Offset;

            res = field.Offset;
            return res;
        }
    }

    L_PRINT(LOG_INFO) << CS_XOR("Schema field NOT found: ") << fieldName
        << CS_XOR(" in class: ") << className;

    return res;
}