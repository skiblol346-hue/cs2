#include "platform/ClientCore.hpp"

#include "runtime/Runtime.hpp"
#include "game/InterfaceRegistry.hpp"

void* get_entity_by_index(int index) {
	if (!ttapp::client::interfaces().m_entity_system)
		return nullptr;
	return ttapp::client::interfaces().m_entity_system->get_base_entity(index);
}

namespace {

bool require_interface(const char* name, const void* value)
{
	if (value)
		return true;

	LOG_ERROR(xorstr_("[interfaces] failed: %s"), name);
	return false;
}

} // namespace

bool InterfaceRegistry::initialize()
{
	const char* client_dll = ttapp::client::modules().m_modules.client_dll.get_name();
	if (!client_dll) {
		LOG_ERROR(xorstr_("[interfaces] failed: client.dll module name"));
		return false;
	}

	using get_input_t = i_csgo_input * (__fastcall*)();
	get_input_t get_input = reinterpret_cast<get_input_t>(
		ttapp::client::patterns().scan_absolute(client_dll, xorstr_("interfaces.get_input"), {
			{ xorstr_("E8 ? ? ? ? 48 8B 56 ? 48 8B C8 E8 ? ? ? ? 4C 89 7E") },
		}, 0x1));
	if (!require_interface(xorstr_("Input"), reinterpret_cast<const void*>(get_input)))
		return false;

	m_csgo_input = get_input();
	if (!require_interface(xorstr_("CInput"), m_csgo_input))
		return false;

	auto entity_system_ref = reinterpret_cast<i_entity_system**>(
		ttapp::client::patterns().scan_absolute(client_dll, xorstr_("interfaces.entity_system"), {
			{ xorstr_("48 8B 0D ? ? ? ? 48 89 7C 24 ? 8B FA C1 EB") },
		}, 0x3));
	if (!entity_system_ref || !*entity_system_ref) {
		LOG_ERROR(xorstr_("[interfaces] failed: Entity System"));
		return false;
	}
	m_entity_system = *entity_system_ref;

	m_schema_system = get_interface<i_schema_system>(&ttapp::client::modules().m_modules.schemasystem_dll, xorstr_("SchemaSystem_001"));
	if (!require_interface(xorstr_("Schema System"), m_schema_system))
		return false;

	m_input_system = get_interface(&ttapp::client::modules().m_modules.input_system, xorstr_("InputSystemVersion001"));
	if (!require_interface(xorstr_("Input System"), m_input_system))
		return false;

	m_source2_client = get_interface<c_source2_client>(&ttapp::client::modules().m_modules.client_dll, xorstr_("Source2Client002"));
	if (!require_interface(xorstr_("Source2Client"), m_source2_client))
		return false;

	m_file_system = get_interface<i_file_system>(&ttapp::client::modules().m_modules.filesystem_stdio, xorstr_("VFileSystem017"));
	if (!require_interface(xorstr_("FileSystem"), m_file_system))
		return false;

	m_panorama = get_interface<i_panorama>(&ttapp::client::modules().m_modules.panorama_dll, xorstr_("PanoramaUIEngine001"));
	if (!require_interface(xorstr_("PanoramaUIEngine"), m_panorama))
		return false;

	return true;
}
