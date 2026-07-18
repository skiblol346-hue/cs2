#pragma once
#include "game/ModuleRegistry.hpp"
#include "game/interfaces/i_csgo_input.hpp"
#include "game/interfaces/i_entity_system.hpp"
#include "game/interfaces/i_schema_system.hpp"
#include "game/interfaces/i_econ_item_system.hpp"
#include "game/interfaces/i_file_system.hpp"
#include "game/interfaces/i_panorama.hpp"
 
template <typename type_t = void*>
inline type_t* get_interface(LoadedModule* dll, const char* name)
{
	if (!dll || !dll->get_name())
		return nullptr;
 
	const HINSTANCE module_handle = GetModuleHandle(dll->get_name());
	if (!module_handle)
		return nullptr;
 
	using create_interface_t = type_t * (*)(const char*, int*);
	const create_interface_t create_interface = reinterpret_cast<create_interface_t>(GetProcAddress(module_handle, "CreateInterface"));
	if (!create_interface)
		return nullptr;
 
	return create_interface(name, nullptr);
};
 
class InterfaceRegistry
{
public:
	i_csgo_input*     m_csgo_input = nullptr;
	i_entity_system*  m_entity_system = nullptr;
	i_schema_system*  m_schema_system = nullptr;
	void*             m_input_system = nullptr;
	c_source2_client* m_source2_client = nullptr;
	i_file_system*    m_file_system = nullptr;
	i_panorama*       m_panorama = nullptr;
 
	bool initialize();
};
