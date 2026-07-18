#pragma once

#include <Windows.h>
#include <memory>

class LoadedModule {
	HMODULE m_dll;
	size_t m_size;
	char* m_name;
public:
	LoadedModule() : m_dll(nullptr), m_size(0), m_name(nullptr) { }
	LoadedModule(const char* name) : m_dll(GetModuleHandleA(name)), m_size(0), m_name(nullptr) {
		if (!m_dll)
			return;

		auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(m_dll);
		auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(m_dll) + dos_header->e_lfanew);
		m_size = nt_header->OptionalHeader.SizeOfImage;

		if (!name)
			return;

		m_name = new char[256];
		strcpy_s(m_name, 256, name);
	}

	uintptr_t get() {
		return reinterpret_cast<uintptr_t>(m_dll);
	}

	const char* get_name() const {
		return m_name;
	}
};

class ModuleRegistry {
private:
	struct modules_t {
		LoadedModule client_dll{};
		LoadedModule input_system{};
		LoadedModule schemasystem_dll{};
		LoadedModule filesystem_stdio{};
		LoadedModule localize_dll{};
		LoadedModule panorama_dll{};

		bool initialize();
	};

public:
	modules_t m_modules{};
};
