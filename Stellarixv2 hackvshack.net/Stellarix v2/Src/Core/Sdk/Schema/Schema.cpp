#include "Schema.hpp"
#include <vector>
#include <Core/Common.hpp>
#include <Core/Utils/Logger/Logger.hpp>
#include <Core/Interfaces/List/CSchemaSystem.hpp>


#define STB_SPRINTF_IMPLEMENTATION


struct SchemaData_t
{
	uint32_t uHashedFieldName = 0x0ULL;
	std::uint32_t uOffset = 0x0U;
};

static std::vector<SchemaData_t> vecSchemaData;

bool SCHEMA::OnInit( )
{
	std::vector<std::string> vecNeededModules = {
		_xor( "client.dll" ), _xor( "engine2.dll" ), _xor( "scenesystem.dll" ),
		_xor( "particles.dll" ), _xor( "tier0.dll" ), _xor( "schemasystem.dll" ),
		_xor( "matchmaking.dll" ), _xor( "animationsystem.dll" )
	};
	for ( const auto& szModule : vecNeededModules )
	{
		if ( !Setup( _xor( L"schema.txt" ), szModule.c_str( ) ) )
			return false;
	}

	return true;
}

bool SCHEMA::Setup( const wchar_t* wszFileName, const char* szModuleName )
{

	//wchar_t wszDumpFilePath[MAX_PATH];
	//if (!GetWorkingPath(wszDumpFilePath))
	//	return false;

	//c_run_time::StringCat(wszDumpFilePath, wszFileName);

//	HANDLE hOutFile = ::CreateFileW(wszDumpFilePath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	//if (hOutFile == INVALID_HANDLE_VALUE)
	//	return false;

	// @todo: maybe remove this redundant? and put it inside CRT::String_t c'tor
//	const std::time_t time = std::time(nullptr);
	//std::tm timePoint;
//	localtime_s(&timePoint, &time);

	//c_run_time::String_t<64> szTimeBuffer(xorstr_("[%d-%m-%Y %T] cs2 | schema dump\n\n"), &timePoint);

	// write current date, time and info
	//::WriteFile(hOutFile, szTimeBuffer.Data(), szTimeBuffer.Length(), nullptr, nullptr);

	CSchemaSystemTypeScope* pTypeScope = GetInterfaceManager( )->GetSchemaSystem( )->FindTypeScopeForModule( szModuleName );
	if ( !pTypeScope )
		return false;

	const int nTableSize = pTypeScope->hashClasses.Count( );

	// allocate memory for elements
	UtlTSHashHandle_t* pElements = new UtlTSHashHandle_t[ nTableSize + 1U ];
	const auto nElements = pTypeScope->hashClasses.GetElements( 0, nTableSize, pElements );

	for ( int i = 0; i < nElements; i++ )
	{
		const UtlTSHashHandle_t hElement = pElements[ i ];

		if ( hElement == 0 )
			continue;

		CSchemaClassBinding* pClassBinding = pTypeScope->hashClasses[ hElement ];
		if ( !pClassBinding )
			continue;

		SchemaClassInfoData_t* pDeclaredClassInfo;
		pTypeScope->FindDeclaredClass( &pDeclaredClassInfo, pClassBinding->szBinaryName );

		if ( !pDeclaredClassInfo )
			continue;

		if ( pDeclaredClassInfo->nFieldSize == 0 )
			continue;

		//c_run_time::String_t<MAX_PATH> szClassBuffer(xorstr_("class %s\n"), pDeclaredClassInfo->szName);
		//::WriteFile(hOutFile, szClassBuffer.Data(), szClassBuffer.Length(), nullptr, nullptr);

		for ( auto j = 0; j < pDeclaredClassInfo->nFieldSize; j++ )
		{
			const SchemaClassFieldData_t* pFields = pDeclaredClassInfo->pFields;
			CRT::String_t<MAX_PATH> szFieldClassBuffer( _xor( "%s->%s" ), pClassBinding->szBinaryName, pFields[ j ].szName );
			// store field info
			vecSchemaData.emplace_back( FNV1A::Hash( szFieldClassBuffer.Data( ) ), pFields[ j ].nSingleInheritanceOffset );

			//c_run_time::String_t<MAX_PATH> szFieldBuffer(xorstr_("    %s %s = 0x%X\n"), pFields[j].pSchemaType->szName, pFields[j].szName, pFields[j].nSingleInheritanceOffset);
			// write field info
			//::WriteFile(hOutFile, szFieldBuffer.Data(), szFieldBuffer.Length(), nullptr, nullptr);
		}
#ifdef _DEBUG
		//	CS2_LOG("dumped  (name: %s total: %i) \n", pDeclaredClassInfo->szName, pDeclaredClassInfo->nFieldSize);
#endif
	}

	// free allocated memory
	delete[ ] pElements;

	// close file
//	::CloseHandle(hOutFile);

	return true;
}

std::uint32_t SCHEMA::GetOffset( const uint32_t uHashedFieldName )
{
	if ( const auto it = std::ranges::find_if( vecSchemaData, [uHashedFieldName]( const SchemaData_t& data )
	{
		return data.uHashedFieldName == uHashedFieldName;
	} );
	it != vecSchemaData.end( ) )
		return it->uOffset;

	L_PRINT( LOG_INFO ) << ( "\nfailed to find offset for field with hash: %i", uHashedFieldName );
	return 0U;
}

// @todo: optimize this, this is really poorly do and can be done much better?
std::uint32_t SCHEMA::GetForeignOffset( const char* szModulenName, const uint32_t uHashedClassName, const uint32_t uHashedFieldName )
{
	CSchemaSystemTypeScope* pTypeScope = GetInterfaceManager( )->GetSchemaSystem( )->FindTypeScopeForModule( szModulenName );
	if ( pTypeScope == nullptr )
		return false;

	const int nTableSize = pTypeScope->hashClasses.Count( );
	// allocate memory for elements
	UtlTSHashHandle_t* pElements = new UtlTSHashHandle_t[ nTableSize + 1U ];
	const auto nElements = pTypeScope->hashClasses.GetElements( 0, nTableSize, pElements );
	std::uint32_t uOffset = 0x0;

	for ( int i = 0; i < nElements; i++ )
	{
		const UtlTSHashHandle_t hElement = pElements[ i ];

		if ( hElement == 0 )
			continue;

		CSchemaClassBinding* pClassBinding = pTypeScope->hashClasses[ hElement ];
		if ( !pClassBinding )
			continue;

		SchemaClassInfoData_t* pDeclaredClassInfo;
		pTypeScope->FindDeclaredClass( &pDeclaredClassInfo, pClassBinding->szBinaryName );

		if ( !pDeclaredClassInfo )
			continue;

		if ( pDeclaredClassInfo->nFieldSize == 0 )
			continue;

		for ( auto j = 0; j < pDeclaredClassInfo->nFieldSize; j++ )
		{
			const SchemaClassFieldData_t* pFields = pDeclaredClassInfo->pFields;
			if ( !pFields )
				continue;

			SchemaClassFieldData_t field = pFields[ j ];
			if ( FNV1A::Hash( pClassBinding->szBinaryName ) == uHashedClassName && FNV1A::Hash( field.szName ) == uHashedFieldName )
				uOffset = field.nSingleInheritanceOffset;
		}
	}

	return uOffset;
}