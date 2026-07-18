#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shlobj_core.h>

#include "config.h"
#include "../core.h"
#include "../utilities/log.h"
#include "../utilities/crt.h"
#include "../utilities/memory.h"

#if defined(CS_CONFIGURATION_BINARY)
#include "../../extensions/binary.h"
#elif defined(CS_CONFIGURATION_JSON)
#include "../../extensions/json.h"
#elif defined(CS_CONFIGURATION_TOML)
#include "../../extensions/toml.h"
#endif

static wchar_t wszConfigurationsPath[MAX_PATH];
#define SAFE_INDEX(idx, vec) if ((idx) >= (vec).size()) return false;
std::size_t C::UserDataType_t::GetSerializationSize() const
{
	std::size_t nTotalDataSize = 0U;

	for (const UserDataMember_t& member : vecMembers)
		nTotalDataSize += sizeof(FNV1A_t[2]) + member.nDataSize;

	return nTotalDataSize;
}



void C::VariableObject_t::SetStorage(const void* pValue)
{
	if (this->nStorageSize <= sizeof(this->storage.uLocal))
	{
		CRT::MemorySet(&this->storage.uLocal, 0U, sizeof(this->storage.uLocal));
		CRT::MemoryCopy(&this->storage.uLocal, pValue, this->nStorageSize);
	}
	else
	{

		if (!this->storage.pHeap)
			this->storage.pHeap = MEM::HeapAlloc(this->nStorageSize);

		CRT::MemorySet(this->storage.pHeap, 0U, this->nStorageSize);
		CRT::MemoryCopy(this->storage.pHeap, pValue, this->nStorageSize);
	}
}

std::size_t C::VariableObject_t::GetSerializationSize() const
{
	std::size_t nSerializationSize = this->nStorageSize;

	switch (this->uTypeHash)
	{
	case FNV1A::HashConst("bool[]"):
	case FNV1A::HashConst("int[]"):
	case FNV1A::HashConst("unsigned int[]"):
	case FNV1A::HashConst("float[]"):
	case FNV1A::HashConst("char[][]"):
		nSerializationSize += sizeof(std::size_t);
		break;
	default:
	{
		for (const UserDataType_t& userType : vecUserTypes)
		{
			if (userType.uTypeHash == this->uTypeHash)
			{
				nSerializationSize = sizeof(std::size_t) + userType.GetSerializationSize();
				break;
			}
		}
		break;
	}
	}

	return nSerializationSize;
}

bool C::Setup(const wchar_t* wszDefaultFileName)
{
	if (!CORE::GetWorkingPath(wszConfigurationsPath))
		return false;

	CRT::StringCat(wszConfigurationsPath, CS_XOR(L"settings\\"));

	if (!::CreateDirectoryW(wszConfigurationsPath, nullptr))
	{
		if (::GetLastError() != ERROR_ALREADY_EXISTS)
		{
			L_PRINT(LOG_ERROR) << CS_XOR("failed to create configurations directory, because one or more intermediate directories don't exist");
			return false;
		}
	}

	AddUserType(FNV1A::HashConst("KeyBind_t"),
		{
			UserDataMember_t{ FNV1A::HashConst("uKey"), FNV1A::HashConst("unsigned int"), &KeyBind_t::uKey },
			UserDataMember_t{ FNV1A::HashConst("nMode"), FNV1A::HashConst("int"), &KeyBind_t::nMode }
		});


	if (!CreateFile(wszDefaultFileName))
		return false;

	Refresh();

	return true;
}


void C::Refresh()
{
	
	for (auto& name : vecFileNames)
		delete[] name;

	vecFileNames.clear();

	wchar_t wszPathFilter[MAX_PATH];
	CRT::StringCat(CRT::StringCopy(wszPathFilter, wszConfigurationsPath),
		CS_XOR(L"*" CS_CONFIGURATION_FILE_EXTENSION));

	WIN32_FIND_DATAW findData;
	if (const HANDLE hFindFile = ::FindFirstFileW(wszPathFilter, &findData);
		hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			wchar_t* name = new wchar_t[CRT::StringLength(findData.cFileName) + 1];
			CRT::StringCopy(name, findData.cFileName);

			vecFileNames.push_back(name);

		} while (::FindNextFileW(hFindFile, &findData));

		::FindClose(hFindFile);
	}
}

void C::AddUserType(const FNV1A_t uTypeHash, const std::initializer_list<UserDataMember_t> vecUserMembers)
{
	if (vecUserMembers.size() == 0U)
		return;

	UserDataType_t userDataType;
	userDataType.uTypeHash = uTypeHash;

	for (const auto& userDataMember : vecUserMembers)
		userDataType.vecMembers.push_back(userDataMember);

	vecUserTypes.emplace_back(CRT::Move(userDataType));
}

bool C::SaveFileVariable(const std::size_t nFileIndex, const VariableObject_t& variable)
{
	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	CRT::StringCat(CRT::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

#if defined(CS_CONFIGURATION_BINARY)
	if (BIN::SaveVariable(wszFilePath, variable))
#elif defined(CS_CONFIGURATION_JSON)
	if (JSON::SaveVariable(wszFilePath, variable))
#elif defined(CS_CONFIGURATION_TOML)
	if (TOML::SaveVariable(wszFilePath, variable))
#endif
	{
		return true;
	}

	return false;
}



bool C::LoadFileVariable(const std::size_t nFileIndex, VariableObject_t& variable)
{
	SAFE_INDEX(nFileIndex, vecFileNames);

	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	CRT::StringCat(CRT::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);


	const std::size_t expected = variable.GetSerializationSize();

#if defined(CS_CONFIGURATION_BINARY)
	if (BIN::LoadVariable(wszFilePath, variable))
#elif defined(CS_CONFIGURATION_JSON)
	if (JSON::LoadVariable(wszFilePath, variable))
#elif defined(CS_CONFIGURATION_TOML)
	if (TOML::LoadVariable(wszFilePath, variable))
#endif
	{
		return true;
	}

	return false;
}

bool C::RemoveFileVariable(const std::size_t nFileIndex, const VariableObject_t& variable)
{
	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	CRT::StringCat(CRT::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

#if defined(CS_CONFIGURATION_BINARY)
	if (BIN::RemoveVariable(wszFilePath, variable))
#elif defined(CS_CONFIGURATION_JSON)
	if (JSON::RemoveVariable(wszFilePath, variable))
#elif defined(CS_CONFIGURATION_TOML)
	if (TOML::RemoveVariable(wszFilePath, variable))
#endif
	{
		return true;
	}

	return false;
}

bool C::CreateFile(const wchar_t* wszFileName)
{
	const wchar_t* wszFileExtension = CRT::StringCharR(wszFileName, L'.');

	const std::size_t nFileNameLength = (wszFileExtension != nullptr ? wszFileExtension - wszFileName : CRT::StringLength(wszFileName));
	wchar_t* wszFullFileName = new wchar_t[nFileNameLength + CRT::StringLength(CS_CONFIGURATION_FILE_EXTENSION) + 1U];

	wchar_t* wszFullFileNameEnd = CRT::StringCopyN(wszFullFileName, wszFileName, nFileNameLength);
	*wszFullFileNameEnd = L'\0';
	CRT::StringCat(wszFullFileNameEnd, CS_XOR(CS_CONFIGURATION_FILE_EXTENSION));

	vecFileNames.push_back(wszFullFileName);

	if (SaveFile(vecFileNames.size() - 1U))
	{
		L_PRINT(LOG_INFO) << CS_XOR("created configuration file: \"") << wszFullFileName << CS_XOR("\"");
		return true;
	}

	L_PRINT(LOG_WARNING) << CS_XOR("failed to create configuration file: \"") << wszFullFileName << CS_XOR("\"");
	return false;
}

// ================== SaveFile ==================

bool C::SaveFile(const std::size_t nFileIndex)
{
	SAFE_INDEX(nFileIndex, vecFileNames);

	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	CRT::StringCat(CRT::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

#if defined(CS_CONFIGURATION_BINARY)
	if (BIN::SaveFile(wszFilePath))
#elif defined(CS_CONFIGURATION_JSON)
	if (JSON::SaveFile(wszFilePath))
#elif defined(CS_CONFIGURATION_TOML)
	if (TOML::SaveFile(wszFilePath))
#endif
	{
		L_PRINT(LOG_INFO) << CS_XOR("saved configuration file: \"") << wszFileName << CS_XOR("\"");
		return true;
	}

	return false;
}


bool C::LoadFile(const std::size_t nFileIndex)
{
	SAFE_INDEX(nFileIndex, vecFileNames);

	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	wchar_t wszFilePath[MAX_PATH];
	CRT::StringCat(CRT::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);


	for (auto& var : vecVariables)
	{
		if (var.nStorageSize <= sizeof(var.storage.uLocal))
			CRT::MemorySet(&var.storage.uLocal, 0, sizeof(var.storage.uLocal));
		else if (var.storage.pHeap)
			CRT::MemorySet(var.storage.pHeap, 0, var.nStorageSize);
	}

#if defined(CS_CONFIGURATION_BINARY)
	if (BIN::LoadFile(wszFilePath))
#elif defined(CS_CONFIGURATION_JSON)
	if (JSON::LoadFile(wszFilePath))
#elif defined(CS_CONFIGURATION_TOML)
	if (TOML::LoadFile(wszFilePath))
#endif
	{
		L_PRINT(LOG_INFO) << CS_XOR("loaded configuration file: \"") << wszFileName << CS_XOR("\"");
		return true;
	}

	L_PRINT(LOG_WARNING) << CS_XOR("failed to load configuration file: \"") << wszFileName << CS_XOR("\"");
	return false;
}

void C::RemoveFile(const std::size_t nFileIndex)
{
	const wchar_t* wszFileName = vecFileNames[nFileIndex];

	// unable to delete default config
	if (CRT::StringCompare(wszFileName, CS_XOR(CS_CONFIGURATION_DEFAULT_FILE_NAME CS_CONFIGURATION_FILE_EXTENSION)) == 0)
	{
		L_PRINT(LOG_WARNING) << CS_XOR("unable to remove default configuration file: \"") << wszFileName << CS_XOR("\"");
		return;
	}

	wchar_t wszFilePath[MAX_PATH];
	CRT::StringCat(CRT::StringCopy(wszFilePath, wszConfigurationsPath), wszFileName);

	if (::DeleteFileW(wszFilePath))
	{
		// erase and free filename from the list
		vecFileNames.erase(vecFileNames.cbegin() + nFileIndex);

		L_PRINT(LOG_INFO) << CS_XOR("removed configuration file: \"") << wszFileName << CS_XOR("\"");
	}
}


std::size_t C::GetVariableIndex(const FNV1A_t uNameHash)
{
	for (std::size_t i = 0U; i < vecVariables.size(); i++)
	{
		if (vecVariables[i].uNameHash == uNameHash)
			return i;
	}

	return C_INVALID_VARIABLE;
}

void ColorPickerVar_t::UpdateRainbow()
{
	if (this->bRainbow)
	{
		const float flTime = static_cast<float>(ImGui::GetTime());
		float arrRainbowColors[] = {
			sin(flTime * this->flRainbowSpeed) * 0.5f + 0.5f,
			sin(flTime * this->flRainbowSpeed * MATH::_PI / 3) * 0.5f + 0.5f,
			sin(flTime * this->flRainbowSpeed * MATH::_PI / 3) * 0.5f + 0.5f,
			this->colValue.Base<COLOR_A>()
		};

		this->colValue = Color_t::FromBase4(arrRainbowColors);
	}
}