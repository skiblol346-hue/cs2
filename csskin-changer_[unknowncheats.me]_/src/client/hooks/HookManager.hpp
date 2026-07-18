#pragma once

class Hook {
	void* m_function = nullptr;
	void* m_detour = nullptr;
	void* m_trampoline = nullptr;
public:
	bool hook(void* target, void* dtr) {
		if (!target || !dtr) {
			LOG_ERROR(xorstr_("[hooks] invalid hook request target=%p detour=%p"), target, dtr);
			return false;
		}

		m_function = target;
		m_detour = dtr;

		MH_STATUS create_status = MH_CreateHook(m_function, m_detour, &m_trampoline);
		if (create_status != MH_OK) {
			LOG_ERROR(xorstr_("[hooks] MH_CreateHook failed: %d"), create_status);
			m_function = nullptr;
			m_detour = nullptr;
			m_trampoline = nullptr;
			return false;
		}

		MH_STATUS enable_status = MH_EnableHook(m_function);
		if (enable_status != MH_OK) {
			LOG_ERROR(xorstr_("[hooks] MH_EnableHook failed: %d"), enable_status);
			MH_DisableHook(m_function);
			MH_RemoveHook(m_function);
			m_function = nullptr;
			m_detour = nullptr;
			m_trampoline = nullptr;
			return false;
		}

		return true;
	}

	bool hook(uintptr_t target, void* dtr) {
		return hook((void*)target, dtr);
	}

	template <typename tOriginal>
	tOriginal get_original() {
		return reinterpret_cast<tOriginal>(m_trampoline);
	}

	bool is_installed() const {
		return m_function && m_detour && m_trampoline;
	}

	void unhook() {
		if (!m_function || !m_detour)
			return;

		MH_DisableHook(m_function);
		m_function = nullptr;
		m_detour = nullptr;
		m_trampoline = nullptr;
	}
};

enum e_frame_stage {
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END
};

class i_csgo_input;

namespace hooks {
	namespace create_move {
		inline Hook m_create_move;
		void __fastcall hk_create_move(i_csgo_input* rcx, int slot, bool active);
	}

	namespace mouse_input_enabled {
		inline Hook m_mouse_input_enabled;
		bool __fastcall hk_mouse_input_enabled(void* rcx);
	}

	namespace enable_cursor {
		inline Hook m_enable_cursor;
		inline bool m_enable_cursor_input = true;
		void* __fastcall hk_enable_cursor(void* rcx, bool active);
	}

	namespace frame_stage_notify {
		inline Hook m_frame_stage_notify;
		void hk_frame_stage_notify(void* source_to_client, int stage);
	}

	namespace fire_event_client_side {
		inline Hook m_fire_event_client_side;
		bool __fastcall hk_fire_event_client_side(void* p_game_event_manager, void* p_game_event);
	}

	namespace level_init {
		inline Hook m_level_init;
		__int64 __fastcall hk_level_init(void* rcx, void* rdx);
	}

	namespace present {
		inline Hook m_present;
		HRESULT hk_present(IDXGISwapChain* swap_chain, unsigned int sync_interval, unsigned int flags);
	}

	namespace resize_buffers {
		inline Hook m_resize_buffers;
		HRESULT hk_resize_buffers(IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);
	}

	namespace create_swap_chain {
		inline Hook m_create_swap_chain;
		HRESULT __stdcall hk_create_swap_chain(IDXGIFactory* factory, IUnknown* device, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swap_chain);
	}

	namespace match_found_handler {
		inline Hook m_match_found_handler;
		void __fastcall hk_match_found_handler();
	}

	namespace panorama_event {
		inline Hook m_panorama_event;
		__int64 __fastcall hk_panorama_event(void* rcx, const char* event_name, void* arg, float value);
	}

	namespace panorama_construct_string {
		inline Hook m_construct_string;
		void* __fastcall hk_construct_string(void* loc, void* panel, const char* text, int64_t ctx, char escapes);
	}

	namespace panorama_get_string {
		inline Hook m_get_string;
		const char* __fastcall hk_get_string(void* self);
	}
}

class HookManager {
public:
	bool initialize();
	void destroy();
};
