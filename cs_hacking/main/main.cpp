#include <Windows.h>
#include <d3d11.h>
#include "../minhook_debug_x64/include/MinHook.h"
#include "../imgui_d11/imgui.h"
#include "../imgui_d11/imgui_impl_win32.h"
#include "../imgui_d11/imgui_impl_dx11.h"
#include <thread>
#include "../gui/gui.h"
#include "../hacking/esp.h"
#include "../cs2_dumper/offsets.hpp"
#include "../cs2_dumper/client_dll.hpp"
#include "../utils/PlayerHelper.hpp"
#include "../utils/pattern_scan.hpp"
#include <cstdint>
#include "../utils/qangle.h"
#include "memhv.h"
#include <utils/CUserCMD.h>
#include "main.h"
static ID3D11Device* g_pd3dDevice = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static ID3D11RenderTargetView* view = nullptr;
static HWND g_hwnd = nullptr;
void* origin_present = nullptr;

using Present = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);



WNDPROC origin_wndProc;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) { return true; }
	return CallWindowProc(origin_wndProc, hwnd, uMsg, wParam, lParam);
}

bool inited = false;
// 全局变量或类成员
ImFont* g_font_default = nullptr;  
ImFont* g_font_icon = nullptr;    

long __stdcall my_present(IDXGISwapChain* _this, UINT a, UINT b) {
	const ImWchar custom_glyph_ranges[] = {
	0x0020, 0x00FF, // 基本ASCII和扩展ASCII
	0x4E00, 0x9FA5, // 常用汉字（简体）
	0xFF00, 0xFFEF, // 全角符号
	0, 0 // 结束标记
	};
	if (!inited) {
		_this->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice);
		g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

		DXGI_SWAP_CHAIN_DESC sd;
		_this->GetDesc(&sd);
		g_hwnd = sd.OutputWindow;

		ID3D11Texture2D* buf{};
		_this->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buf);
		g_pd3dDevice->CreateRenderTargetView(buf, nullptr, &view);
		buf->Release();

		origin_wndProc = (WNDPROC)SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplWin32_Init(g_hwnd);
		ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);
		g_font_default = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		g_font_icon = io.Fonts->AddFontFromFileTTF("D:\\EdgeDownload\\undefeated_[unknowncheats.me]_\\undefeated.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		inited = true;
	}


	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	draw_Menu();
	draw_esp();
	test();
	ImGui::EndFrame();

	ImGui::Render();
	g_pd3dContext->OMSetRenderTargets(1, &view, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	return ((Present)origin_present)(_this, a, b);
}




static bool(__fastcall* fnOriginalCreateMove)(void*, int, CUserCMD*) = nullptr;
static bool __fastcall hkCreateMove(void* pCSGOInput, int nSlot, CUserCMD* pcmd) {
	bool bResult = fnOriginalCreateMove(pCSGOInput, nSlot, pcmd);

	
		return bResult;
}

DWORD create(void*) {
	const unsigned level_count = 2;
	D3D_FEATURE_LEVEL levels[level_count] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = GetForegroundWindow();
	sd.SampleDesc.Count = 1;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	auto hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		levels,
		level_count,
		D3D11_SDK_VERSION,
		&sd,
		&g_pSwapChain,
		&g_pd3dDevice,
		nullptr,
		nullptr);

	if (g_pSwapChain) {
		auto vtable_ptr = (void***)(g_pSwapChain);
		auto vtable = *vtable_ptr;
		auto present = vtable[8];
		MH_Initialize();
		MH_CreateHook(present, my_present, &origin_present);
		MH_EnableHook(present);
		g_pd3dDevice->Release();
		g_pSwapChain->Release();
	}

	AllocConsole();
	FILE* file;
	freopen_s(&file, "CONOUT$", "w", stdout);


	const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));

	void* pCCSGOInput = reinterpret_cast<void*>(client + cs2_dumper::offsets::client_dll::dwCSGOInput);

	void* pfnCreateMove = (*reinterpret_cast<void***>(pCCSGOInput))[21];

	MH_CreateHook(pfnCreateMove, hkCreateMove, reinterpret_cast<void**>(&fnOriginalCreateMove));
	MH_EnableHook(pfnCreateMove);

	return 0;



}







BOOL WINAPI DllMain(HMODULE hModule, 
	DWORD ul_reason_for_call, 
	LPVOID lpReserved
	) 
{
	if (ul_reason_for_call == 1) {
		CreateThread(NULL, 0, create, NULL, 0, NULL);
		//CreateThread(NULL, 0, tiggerbot, NULL, 0, NULL);
	}
	return TRUE;
}






