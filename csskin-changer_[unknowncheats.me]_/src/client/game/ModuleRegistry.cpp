#include "platform/ClientCore.hpp"
#include "game/ModuleRegistry.hpp"

namespace {

bool require_module(LoadedModule& field, const char* name)
{
	field = LoadedModule(name);
	if (!field.get()) {
		LOG_ERROR(xorstr_("[modules] missing required module: %s"), name);
		return false;
	}

	return true;
}

} // namespace

bool ModuleRegistry::modules_t::initialize() {
	if (!require_module(client_dll, xorstr_("client.dll")))
		return false;
	if (!require_module(input_system, xorstr_("inputsystem.dll")))
		return false;
	if (!require_module(schemasystem_dll, xorstr_("schemasystem.dll")))
		return false;
	if (!require_module(filesystem_stdio, xorstr_("filesystem_stdio.dll")))
		return false;
	if (!require_module(panorama_dll, xorstr_("panorama.dll")))
		return false;

	localize_dll = LoadedModule(xorstr_("localize.dll"));
	if (!localize_dll.get())
		LOG_ERROR(xorstr_("[modules] optional module missing: localize.dll; names will use raw tokens"));

	return true;
}
