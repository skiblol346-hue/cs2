#pragma once
#include <memory>
#include <Fnv1a/fnv1a.h>


#define SCHEMA_ADD_OFFSET(TYPE, NAME, OFFSET)                                                                 \
	[[nodiscard]] __forceinline std::add_lvalue_reference_t<TYPE> NAME()                                          \
	{                                                                                                         \
		static const std::uint32_t uOffset = OFFSET;                                                          \
		return *reinterpret_cast<std::add_pointer_t<TYPE>>(reinterpret_cast<std::uint8_t*>(this) + (uOffset)); \
	}

#define SCHEMA_ADD_POFFSET(TYPE, NAME, OFFSET)                                                               \
	[[nodiscard]] __forceinline std::add_pointer_t<TYPE> NAME()                                                  \
	{                                                                                                        \
		const static std::uint32_t uOffset = OFFSET;                                                         \
		return reinterpret_cast<std::add_pointer_t<TYPE>>(reinterpret_cast<std::uint8_t*>(this) + (uOffset)); \
	}

#define SCHEMA_ADD_FIELD_OFFSET(TYPE, NAME, FIELD, ADDITIONAL) SCHEMA_ADD_OFFSET(TYPE, NAME, SCHEMA::GetOffset(FNV1A::HashConst(FIELD)) + ADDITIONAL)

#define SCHEMA_ADD_FIELD(TYPE, NAME, FIELD) SCHEMA_ADD_FIELD_OFFSET(TYPE, NAME, FIELD, 0U)


// @todo: dump enums?
namespace SCHEMA
{
	bool OnInit( );

	// store the offset of the field in the class
	// dump stored data to file
	bool Setup( const wchar_t* wszFileName, const char* szModuleName );

	/* @section: get */
	// get offset of the field in the class
	// @note: only client.dll class & fields
	[[nodiscard]] std::uint32_t GetOffset( const uint32_t uHashedFieldName );

	// get foregin offset from other .dll
	[[nodiscard]] std::uint32_t GetForeignOffset( const char* szModulenName, const uint32_t uHashedClassName, const uint32_t uHashedFieldName );
}