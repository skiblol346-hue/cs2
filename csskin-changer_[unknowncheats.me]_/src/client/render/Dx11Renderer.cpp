#include "platform/ClientCore.hpp"

#include "runtime/Runtime.hpp"
#include "render/Dx11Renderer.hpp"
#include "ui/Menu.hpp"
#include "hooks/HookManager.hpp"
#include <windowsx.h>
#include "config/ConfigStore.hpp"
#include "imgui_internal.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")


IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool g_first_time_opened = true;
static bool g_force_refresh_needed = false;
static void force_cursor_refresh(HWND hwnd)
{
	while (::ShowCursor(TRUE) < 0);
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));

	::ReleaseCapture();

	::SendMessageA(hwnd, WM_ACTIVATE, WA_INACTIVE, 0);
	::SendMessageA(hwnd, WM_ACTIVATE, WA_ACTIVE, 0);

	::InvalidateRect(hwnd, NULL, FALSE);
}
static LRESULT CALLBACK wnd_proc_hook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	
	if (ttapp::client::menu().is_waiting_for_key() && msg == WM_KEYDOWN)
	{
		
		if (wparam == VK_ESCAPE)
		{
			ttapp::client::menu().set_waiting_for_key(false);
		}
		else
		{
			
			ttapp::client::config().misc.m_menu_key = (int)wparam;
			ttapp::client::menu().set_waiting_for_key(false);
			extern void save_last_state(const std::string & config_name);
			save_last_state(ttapp::client::config_store().m_selected_config);
		}

		return true; 
	}


	if (msg == WM_KEYDOWN && wparam == ttapp::client::config().misc.m_menu_key)
	{
		ttapp::client::menu().m_opened = !ttapp::client::menu().m_opened;
		const bool now_open = ttapp::client::menu().m_opened;
		auto& io = ImGui::GetIO();

		io.MouseDrawCursor = now_open;

		if (!now_open) {
			
			ImGui::ClosePopupsOverWindow(nullptr, false);
			io.MouseDown[0] = io.MouseDown[1] = io.MouseDown[2] = false;
			force_cursor_refresh(hwnd);
		}
		else {
			
			while (::ShowCursor(FALSE) >= 0);
		}

		
		using fn = decltype(&hooks::enable_cursor::hk_enable_cursor);
		auto original = hooks::enable_cursor::m_enable_cursor.get_original<fn>();
		if (original) {
			
			const bool requested_game_cursor_state = now_open ? false : true;
			__try {
				original(ttapp::client::interfaces().m_input_system, requested_game_cursor_state);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}
		}
		return true;
	}

	
	if (ttapp::client::menu().m_opened) {
		// Scale mouse coordinates in lParam to match the backbuffer scaling in ImGui
		if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST && msg != WM_MOUSEWHEEL && msg != WM_MOUSEHWHEEL) {
			const short x = (short)LOWORD(lparam);
			const short y = (short)HIWORD(lparam);
			const float scale_x = ttapp::client::renderer().get_mouse_scale_x();
			const float scale_y = ttapp::client::renderer().get_mouse_scale_y();
			lparam = MAKELPARAM((WORD)(x * scale_x), (WORD)(y * scale_y));
		}

		ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);

		if (msg == WM_SETCURSOR) return 1;
		if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) return true;

		auto& io = ImGui::GetIO();
		if (io.WantCaptureKeyboard || io.WantTextInput) {
			if (msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_CHAR ||
				msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) return true;
		}
	}

	
	return CallWindowProcA(ttapp::client::renderer().get_wnd_proc_original(), hwnd, msg, wparam, lparam);
}
bool Dx11Renderer::initialize() {

	WNDCLASSEXA wc{};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = DefWindowProcA;
	wc.hInstance = GetModuleHandleA(nullptr);
	wc.lpszClassName = "ttapp_dummy_window";
	const ATOM dummy_class = RegisterClassExA(&wc);
	const bool dummy_class_registered = dummy_class != 0;
	const DWORD dummy_class_error = dummy_class_registered ? ERROR_SUCCESS : GetLastError();
	if (!dummy_class_registered && dummy_class_error != ERROR_CLASS_ALREADY_EXISTS) {
		LOG_ERROR(xorstr_("[renderer] dummy window class registration failed: %lu"), dummy_class_error);
		return false;
	}

	HWND dummy_hwnd = CreateWindowExA(0, wc.lpszClassName, "", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1,
		nullptr, nullptr, wc.hInstance, nullptr);
	if (!dummy_hwnd) {
		LOG_ERROR(xorstr_("[renderer] dummy window creation failed: %lu"), GetLastError());
		if (dummy_class_registered)
			UnregisterClassA(wc.lpszClassName, wc.hInstance);
		return false;
	}

	DXGI_SWAP_CHAIN_DESC desc{};
	desc.BufferCount = 1;
	desc.BufferDesc.Width = 1;
	desc.BufferDesc.Height = 1;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow = dummy_hwnd;
	desc.SampleDesc.Count = 1;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	IDXGISwapChain* dummy_chain = nullptr;
	ID3D11Device* dummy_device = nullptr;
	ID3D11DeviceContext* dummy_ctx = nullptr;
	D3D_FEATURE_LEVEL feature_level;
	const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, 0, levels, _countof(levels), D3D11_SDK_VERSION,
		&desc, &dummy_chain, &dummy_device, &feature_level, &dummy_ctx);

	if (FAILED(hr) || !dummy_chain) {
		LOG_ERROR(xorstr_("[renderer] dummy swap chain creation failed: 0x%08X"), hr);
		if (dummy_hwnd) DestroyWindow(dummy_hwnd);
		if (dummy_class_registered)
			UnregisterClassA(wc.lpszClassName, wc.hInstance);
		return false;
	}

	m_present_address = vmt::get_v_method(dummy_chain, 8);
	m_resize_buffers_address = vmt::get_v_method(dummy_chain, 13);

	IDXGIDevice* dxgi_device = nullptr;
	if (SUCCEEDED(dummy_chain->GetDevice(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device))) && dxgi_device) {
		IDXGIAdapter* adapter = nullptr;
		if (SUCCEEDED(dxgi_device->GetAdapter(&adapter)) && adapter) {
			IDXGIFactory* factory = nullptr;
			if (SUCCEEDED(adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory))) && factory) {
				m_create_swap_chain_address = vmt::get_v_method(factory, 10);
				factory->Release();
			}
			adapter->Release();
		}
		dxgi_device->Release();
	}

	if (dummy_ctx)    dummy_ctx->Release();
	if (dummy_device) dummy_device->Release();
	if (dummy_chain)  dummy_chain->Release();
	if (dummy_hwnd)   DestroyWindow(dummy_hwnd);
	if (dummy_class_registered)
		UnregisterClassA(wc.lpszClassName, wc.hInstance);

	if (!m_present_address) {
		LOG_ERROR(xorstr_("[renderer] missing target: IDXGISwapChain::Present"));
		return false;
	}
	if (!m_resize_buffers_address) {
		LOG_ERROR(xorstr_("[renderer] missing target: IDXGISwapChain::ResizeBuffers"));
		return false;
	}
	if (!m_create_swap_chain_address) {
		LOG_ERROR(xorstr_("[renderer] missing target: IDXGIFactory::CreateSwapChain"));
		return false;
	}

	return true;
}

void Dx11Renderer::try_resolve_late_hooks(IDXGISwapChain*) {

}

void Dx11Renderer::uninitialize() {
	if (m_wnd_proc_original && m_window) {
		SetWindowLongPtrA(m_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_wnd_proc_original));
		m_wnd_proc_original = nullptr;
	}

	destroy_render_target();

	if (m_imgui_initialized) {
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		m_imgui_initialized = false;
	}

	if (m_device_context) { m_device_context->Release(); m_device_context = nullptr; }
	if (m_device) { m_device->Release();         m_device = nullptr; }
	m_swap_chain = nullptr;
	m_window = nullptr;
	m_started = false;
	m_initial_cursor_synced = false;
	m_pending_resize = false;
	m_last_dpi_scale = 0.0f;
	m_mouse_scale_x = 1.0f;
	m_mouse_scale_y = 1.0f;
	m_backbuffer_width = 0;
	m_backbuffer_height = 0;
	m_client_width = 0;
	m_client_height = 0;
}

void Dx11Renderer::create_render_target() {
	if (!m_swap_chain || !m_device)
		return;

	if (m_render_target) {
		m_render_target->Release();
		m_render_target = nullptr;
	}

	ID3D11Texture2D* back_buffer = nullptr;
	if (SUCCEEDED(m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer))) && back_buffer) {
		m_device->CreateRenderTargetView(back_buffer, nullptr, &m_render_target);
		back_buffer->Release();
	}
}

void Dx11Renderer::destroy_render_target() {
	if (m_render_target) {
		m_render_target->Release();
		m_render_target = nullptr;
	}
}

void Dx11Renderer::invalidate_device_objects() {
	if (m_imgui_initialized && ImGui::GetCurrentContext())
		ImGui_ImplDX11_InvalidateDeviceObjects();
}

void Dx11Renderer::on_resize_buffers() {
	m_pending_resize = true;
	m_backbuffer_width = 0;
	m_backbuffer_height = 0;
	m_client_width = 0;
	m_client_height = 0;
	m_mouse_scale_x = 1.0f;
	m_mouse_scale_y = 1.0f;
}

void Dx11Renderer::sync_display_metrics(IDXGISwapChain* swap_chain) {
	if (!m_imgui_initialized)
		return;

	UINT width = 0;
	UINT height = 0;
	UINT client_width = 0;
	UINT client_height = 0;

	if (swap_chain) {
		ID3D11Texture2D* back_buffer = nullptr;
		if (SUCCEEDED(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer))) && back_buffer) {
			D3D11_TEXTURE2D_DESC desc{};
			back_buffer->GetDesc(&desc);
			width = desc.Width;
			height = desc.Height;
			back_buffer->Release();
		}
	}

	if (m_window) {
		RECT rect{};
		if (GetClientRect(m_window, &rect)) {
			client_width = static_cast<UINT>(rect.right - rect.left);
			client_height = static_cast<UINT>(rect.bottom - rect.top);
		}
	}

	if ((width == 0 || height == 0) && client_width > 0 && client_height > 0) {
		width = client_width;
		height = client_height;
	}

	if (width == 0 || height == 0)
		return;

	m_backbuffer_width = width;
	m_backbuffer_height = height;
	m_client_width = client_width;
	m_client_height = client_height;
	m_mouse_scale_x = client_width > 0 ? (float)width / (float)client_width : 1.0f;
	m_mouse_scale_y = client_height > 0 ? (float)height / (float)client_height : 1.0f;

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)width, (float)height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	update_dpi_scale();
}

void Dx11Renderer::sync_mouse_position() {
	if (!m_imgui_initialized || !m_window || m_backbuffer_width == 0 || m_backbuffer_height == 0)
		return;

	if (m_client_width == 0 || m_client_height == 0)
		return;

	POINT pos{};
	if (!GetCursorPos(&pos) || !ScreenToClient(m_window, &pos))
		return;

	ImGui::GetIO().AddMousePosEvent(
		(float)pos.x * m_mouse_scale_x,
		(float)pos.y * m_mouse_scale_y
	);
}

void Dx11Renderer::apply_render_viewport() {
	if (!m_device_context || m_backbuffer_width == 0 || m_backbuffer_height == 0)
		return;

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)m_backbuffer_width;
	viewport.Height = (float)m_backbuffer_height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_device_context->RSSetViewports(1, &viewport);
}

void Dx11Renderer::update_dpi_scale() {
	if (!m_imgui_initialized)
		return;

	UINT height = m_backbuffer_height;
	if (height == 0 && m_window) {
		RECT rect{};
		if (GetClientRect(m_window, &rect))
			height = static_cast<UINT>(rect.bottom - rect.top);
	}

	if (height <= 0)
		return;

	float scale = (float)height / 1080.0f;
	if (scale < 0.5f) scale = 0.5f;
	if (scale > 4.0f) scale = 4.0f;

	if (std::abs(scale - m_last_dpi_scale) > 0.01f) {
		ttapp::client::menu().rebuild_fonts(scale);
		m_last_dpi_scale = scale;
	}
}

void Dx11Renderer::start_frame(IDXGISwapChain* swap_chain)
{

	if (!m_started)
	{
		m_swap_chain = swap_chain;

		swap_chain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&m_device));
		m_device->GetImmediateContext(&m_device_context);

		DXGI_SWAP_CHAIN_DESC desc;
		swap_chain->GetDesc(&desc);
		m_window = desc.OutputWindow;

		m_wnd_proc_original = reinterpret_cast<WNDPROC>(
			SetWindowLongPtrA(m_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wnd_proc_hook)));

		create_render_target();

		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(m_window);
		ImGui_ImplDX11_Init(m_device, m_device_context);

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = m_window;

		m_imgui_initialized = true;

		sync_display_metrics(swap_chain);
		if (m_last_dpi_scale == 0.0f)
			update_dpi_scale();

		force_cursor_refresh(m_window);

		m_started = true;

	
	}

	m_swap_chain = swap_chain;

	if (m_pending_resize) {
		create_render_target();
		m_pending_resize = false;
	}

	if (!m_render_target)
		create_render_target();

	sync_display_metrics(swap_chain);


}

void Dx11Renderer::new_frame() {
	if (!m_imgui_initialized)
		return;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	sync_display_metrics(m_swap_chain);
	sync_mouse_position();
	ImGui::NewFrame();
}

void Dx11Renderer::end_frame() {
	if (!m_imgui_initialized)
		return;

	ImGui::EndFrame();
	ImGui::Render();
	apply_render_viewport();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
