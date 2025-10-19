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
		io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		inited = true;
	}


	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	draw_Menu();
	draw_esp();
	ImGui::EndFrame();

	ImGui::Render();
	g_pd3dContext->OMSetRenderTargets(1, &view, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	return ((Present)origin_present)(_this, a, b);
}




static bool(__fastcall* fnOriginalCreateMove)(void*, int, CUserCMD*) = nullptr;
static bool __fastcall hkCreateMove(void* pCSGOInput, int nSlot, CUserCMD* pcmd) {
	bool bResult = fnOriginalCreateMove(pCSGOInput, nSlot, pcmd);

	//bhop(pcmd);
	/*const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));

	auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);


	if (!local_ctrl)
	{
		return bResult;
	}
	auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
	if (local_hpawn == 0xFFFFFFFF)
	{
		return bResult;
	}

	auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);
	if (!localpawn)
	{
		return bResult;
	}
	auto localteam = *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
	if (!localteam)
	{
		return bResult;
	}
	for (int i = 0; i < 64; i++) {
		auto player_co = GetBaseEntity(i, client);
		if (!player_co)
			continue;
		if (player_co == localpawn) {
			continue;
		}
		auto player_hpawn = *reinterpret_cast<uint32_t*>(player_co + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
		if (player_hpawn == 0xFFFFFFFF)
		{
			continue;
		}

		auto player_pawn = GetBaseEntityFromHandle(player_hpawn, client);
		if (!player_pawn)
		{
			continue;
		}

		auto player_team = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
		if (localteam == player_team)
			continue;

		/*std::string name;
		uintptr_t temp = *reinterpret_cast<uintptr_t*>(player_co + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName);
		if (temp) {
			char buff[50]{};
			HV::ReadMemory(temp, (ULONG64)&buff, sizeof(buff));  // sizeof(buff) == 50
			name = std::string(buff);
		}
		printf("player name: %s\n", name);*/
		/*auto weapon_id = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_pClippingWeapon +
			cs2_dumper::schemas::client_dll::C_Chicken::m_AttributeManager + 
			cs2_dumper::schemas::client_dll::C_AttributeContainer::m_Item + 
			cs2_dumper::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex);
		printf("weapon id: %d\n", weapon_id);
		auto weaponServices = *reinterpret_cast<uintptr_t*>(player_pawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_pWeaponServices);
		if (!weaponServices) {
			continue;
		}
		
		// 获取当前武器句柄（使用与偏移匹配的类型）
		// 若m_hActiveWeapon实际是32位，用uint32_t；若是64位，用uint64_t
		uint32_t activeWeaponHandle = *reinterpret_cast<uint32_t*>(weaponServices + cs2_dumper::schemas::client_dll::CPlayer_WeaponServices::m_hActiveWeapon);
		// 检查无效句柄（32位无效值）
		if (activeWeaponHandle == 0xFFFFFFFF) {
			continue;
		}

		// 解析武器实体地址
		uintptr_t weaponBase = GetBaseEntityFromHandle(activeWeaponHandle, client);
		if (!weaponBase) {
			continue;
		}

		// 成功获取武器实体
		auto player_iAmmo = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_iClip1);
		printf("player armor: %d\n", player_iAmmo);
		auto player_pReserveAmmo1 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_pReserveAmmo);
		printf("player reserve ammo: %d\n", player_pReserveAmmo1);
		auto player_weaponID = *reinterpret_cast<uint16_t*>(weaponBase + cs2_dumper::schemas::client_dll::C_EconEntity::m_AttributeManager + cs2_dumper::schemas::client_dll::C_AttributeContainer::m_Item + cs2_dumper::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex);
		printf("player_weaponID: %d\n", player_weaponID);*/
		/*auto player_name = *reinterpret_cast<uintptr_t*>(player_pawn + cs2_dumper::schemas::client_dll::CBasePlayerController::m_iszPlayerName);
		if (!player_name)
			continue;
		SetConsoleOutputCP(CP_UTF8);
		printf("player name: %s\n", player_name);*/
	//}
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





/*//静默自瞄
void* GOrigin = nullptr;
int64_t MyHookFunc(int* a1, int64_t a2, char a3, int64_t arg1, int64_t arg2, int64_t arg3) {

	if (cs::aimbot::aimbot && GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
		auto h = CS2Helper::Instance();
		auto localPtr = h.getLocalPlayerPawn();
		auto myTid = CS2Helper::getTeamNum(localPtr);
		auto myAngle = h.getViewAngle<QAngle_t>();
		uint64_t closedTarget{ INFINITE };
		QAngle_t targetAngle;
		bool findOut{ false };
		// find target loop
		for (int i = 0; i < 64; i++) {
			auto targetPtr = h.getPlayer(i);
			if (!targetPtr)
				continue;

			//not local
			if (targetPtr == localPtr)
				continue;

			// same team
			auto targetTid = CS2Helper::getTeamNum(targetPtr);
			if (targetTid == myTid)
				continue;

			// life
			if (CS2Helper::getLifeState(targetPtr))
				continue;

			auto getAngle = AimBot::GetTargetAngle(targetPtr, localPtr);

			auto diff = (getAngle - myAngle).Length2D();
			if (diff < closedTarget) {
				findOut = true;
				closedTarget = diff;
				targetAngle = getAngle;
			}

		}

		if (findOut) {
			//silent aim  is psilent ?
			memcpy(&a1[4], &targetAngle.x, sizeof(float)); //set Pitch
			memcpy(&a1[5], &targetAngle.y, sizeof(float)); //set Yaw
			//normal aim
			//h.writeViewAngle<QAngle_t>(targetAngle);
		}

	}



	using FN_V = int64_t(*)(int* a1, int64_t a2, char a3, int64_t arg1, int64_t arg2, int64_t arg3);
	return ((FN_V)GOrigin)(a1, a2, a3, arg1, arg2, arg3);
}

void* GModule;
auto hookTarget = Scanner::PatternScan("client.dll", "4C 89 4C 24 20 55 53 57 41 56 48 8D 6C 24 D1");

*/

BOOL WINAPI DllMain(HMODULE hModule, 
	DWORD ul_reason_for_call, 
	LPVOID lpReserved
	) 
{
	if (ul_reason_for_call == 1) {
		CreateThread(NULL, 0, create, NULL, 0, NULL);
		Sleep(2000);
		//CreateThread(NULL, 0, tiggerbot, NULL, 0, NULL);
	}
	/*if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		GModule = hModule;
		MH_Initialize();
		MH_CreateHook(hookTarget, MyHookFunc, &GOrigin);
		MH_EnableHook(hookTarget);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		MH_DisableHook(hookTarget);
		MH_Uninitialize();
	}*/
	return TRUE;
}






