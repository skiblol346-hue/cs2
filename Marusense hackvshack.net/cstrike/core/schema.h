#pragma once


#include "../common.h"

// used: fnv-1a hash
#include "../utilities/fnv1a.h"

#include <unordered_map>

class ISchemaSystem;
struct CSchemaClassInfo;
struct CSchemaClassField;
struct CSchemaDeclaredClass;
struct CSchemaDeclaredClassEntry;
struct CSchemaTypeScope;

#define SCHEMA_ADD_OFFSET(TYPE, NAME, OFFSET)                                                                 \
	[[nodiscard]] CS_INLINE std::add_lvalue_reference_t<TYPE> NAME()                                          \
	{                                                                                                         \
		static const std::uint32_t uOffset = OFFSET;                                                          \
		return *reinterpret_cast<std::add_pointer_t<TYPE>>(reinterpret_cast<std::uint8_t*>(this) + (uOffset)); \
	}

#define SCHEMA_ADD_POFFSET(TYPE, NAME, OFFSET)                                                               \
	[[nodiscard]] CS_INLINE std::add_pointer_t<TYPE> NAME()                                                  \
	{                                                                                                        \
		const static std::uint32_t uOffset = OFFSET;                                                         \
		return reinterpret_cast<std::add_pointer_t<TYPE>>(reinterpret_cast<std::uint8_t*>(this) + (uOffset)); \
	}

#define SCHEMA_ADD_FIELD(type, name, className, fieldName) \
type& name() { \
    static uint32_t offset = SCHEMA::Get("client.dll", className, fieldName); \
    return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); \
}

#define SCHEMA_ADD_FIELD_EX(type, name, className, fieldName, extraOffset) \
type& name() { \
    static uint32_t offset = SCHEMA::Get("client.dll", className, fieldName); \
    return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset + extraOffset); \
}

namespace SCHEMA
{
	inline std::unordered_map<std::string, uint32_t> m_fieldCache;

	uint32_t Get(const char*, const char*, const char*);

	bool FindDeclaredClass(CSchemaTypeScope*, const char*, CSchemaDeclaredClass**);
}