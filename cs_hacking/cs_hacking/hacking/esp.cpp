#include <Windows.h>
#include <cstdint>
#include "../cs2_dumper/offsets.hpp"
#include "../cs2_dumper/client_dll.hpp"
#include "../utils/Vector.h"
#include <optional>
#include "../imgui_d11/imgui.h"
#include "../gui/gui.h"
#include "esp.h"
#include "../utils/Format.h"
#include "Random.h"
#include "../imgui_d11/imgui_internal.h"
#include <map>
#include "VisCheck.h"
#include "mouse.h"
#include <iostream>
#include <unordered_map>
#include "Helper.h"
#include "math.h"
#include <cs2_dumper/buttons.hpp>
#include "main/main.h"
uintptr_t GetBaseEntity(int index, uintptr_t client) {
	auto entListBase = *reinterpret_cast<std::uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwEntityList);
	if (entListBase == 0) {
		return 0;
	}
	auto entitylistbase = *reinterpret_cast<std::uintptr_t*>(entListBase + 0x8 * (index >> 9) + 16);
	if (entitylistbase == 0) {
		return 0;
	}
	return *reinterpret_cast<std::uintptr_t*>(entitylistbase + (0x70 * (index & 0x1FF)));
}

uintptr_t GetBaseEntityFromHandle(uint32_t uHandle, uintptr_t client) {
	auto entListBase = *reinterpret_cast<std::uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwEntityList);
	if (entListBase == 0) {
		return 0;
	}

	const int nIndex = uHandle & 0x7FFF;

	auto entitylistbase = *reinterpret_cast<std::uintptr_t*>(entListBase + 8 * (nIndex >> 9) + 16);
	if (entitylistbase == 0) {
		return 0;
	}
	return *reinterpret_cast<std::uintptr_t*>(entitylistbase + 0x70 * (nIndex & 0x1FF));
}


bool WorldToScreen(Vector3 pWorldPos, Vector3& pScreenPos, float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight)
{
	float matrix2[4][4];

	memcpy(matrix2, pMatrixPtr, 16 * sizeof(float));

	const float mX{ pWinWidth / 2 };
	const float mY{ pWinHeight / 2 };

	const float w{
		matrix2[3][0] * pWorldPos.x +
		matrix2[3][1] * pWorldPos.y +
		matrix2[3][2] * pWorldPos.z +
		matrix2[3][3] };

	if (w < 0.65f) return false;

	const float x{
		matrix2[0][0] * pWorldPos.x +
		matrix2[0][1] * pWorldPos.y +
		matrix2[0][2] * pWorldPos.z +
		matrix2[0][3] };

	const float y{
		matrix2[1][0] * pWorldPos.x +
		matrix2[1][1] * pWorldPos.y +
		matrix2[1][2] * pWorldPos.z +
		matrix2[1][3] };

	pScreenPos.x = (mX + mX * x / w);
	pScreenPos.y = (mY - mY * y / w);
	pScreenPos.z = 0;

	return true;


}





Vector3 BonePos(uintptr_t addr, int32_t index)
{
	int32_t d = 32 * index;
	uintptr_t address{};
	address = *reinterpret_cast<uintptr_t*>(addr + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
	if (!address)
	{
		return Vector3();
	}
	auto BoneArray = cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80;
	address = *reinterpret_cast<uintptr_t*>(address + BoneArray);
	if (!address)
	{
		return Vector3();
	}
	return *reinterpret_cast<Vector3*>(address + d);
}


std::optional<Vector3> GetEyePos(uintptr_t addr) noexcept {
	auto* Origin = reinterpret_cast<Vector3*>(addr + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
	auto* ViewOffset = reinterpret_cast<Vector3*>(addr + cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset);

	Vector3 LocalEye = *Origin + *ViewOffset;
	if (!std::_Is_finite(LocalEye.x) || !std::_Is_finite(LocalEye.y) || !std::_Is_finite(LocalEye.z))
		return std::nullopt;
	if (LocalEye.Length() < 0.1f)
		return std::nullopt;
	return LocalEye;
}

void DrawLine(std::vector<Vector3> list, ImColor Color, float* Matrix)
{
	Vector3 drawpos;
	std::vector<Vector3>Drawlist{};
	for (int i = 0; i < list.size(); ++i)
	{
		if (!WorldToScreen(list[i], drawpos, Matrix, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y))
			continue;

		Drawlist.push_back(drawpos);

	}

	for (int i = 1; i < Drawlist.size(); ++i) {
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(Drawlist[i].x, Drawlist[i].y), ImVec2(Drawlist[i - 1].x, Drawlist[i - 1].y), Color);
	}
}

void Bone_Start(uintptr_t pawn, ImColor BoneColor, float* Matrix) {
	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::head));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::neck_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::spine_2));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::pelvis));

	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::neck_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::arm_upper_L));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::arm_lower_L));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::hand_L));

	DrawLine(BoneDrawList, BoneColor, Matrix);


	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::neck_0));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::arm_upper_R));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::arm_lower_R));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::hand_R));

	DrawLine(BoneDrawList, BoneColor, Matrix);

	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::pelvis));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::leg_upper_L));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::leg_lower_L));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::ankle_L));

	DrawLine(BoneDrawList, BoneColor, Matrix);


	BoneDrawList.clear();
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::pelvis));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::leg_upper_R));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::leg_lower_R));
	BoneDrawList.push_back(BonePos(pawn, Bone_Base::BoneIndex::ankle_R));

	DrawLine(BoneDrawList, BoneColor, Matrix);


}



// 提取绘制函数：参数为绘制列表、坐标、宽高、颜色
void DrawFilledRoundedBox(ImDrawList* DrawList, float x, float y, float width, float height, ImColor color) {
	ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All;
	ImVec2 a = ImVec2(x, y);
	ImVec2 b = ImVec2(x + width, y + height);
	float Rounding = RandomPara<float>(0.0f, 3.0f);

	Rounding = ImMin<float>(Rounding, fabsf(width) * (((rounding_corners & ImDrawCornerFlags_Top) == ImDrawCornerFlags_Top) || ((rounding_corners & ImDrawCornerFlags_Bot) == ImDrawCornerFlags_Bot) ? 0.5f : 1.0f) - 1.0f);
	Rounding = ImMin<float>(Rounding, fabsf(height) * (((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f) - 1.0f);

	if (Rounding <= 0.0f || rounding_corners == 0) {
		DrawList->PathLineTo(a);
		DrawList->PathLineTo(ImVec2(b.x, a.y));
		DrawList->PathLineTo(b);
		DrawList->PathLineTo(ImVec2(a.x, b.y));
	}
	else {
		DrawList->PathArcTo(ImVec2(a.x + Rounding, a.y + Rounding), Rounding, IM_PI, IM_PI / 2.f * 3.f);
		DrawList->PathArcTo(ImVec2(b.x - Rounding, a.y + Rounding), Rounding, IM_PI / 2.f * 3.f, IM_PI * 2.f);
		DrawList->PathArcTo(ImVec2(b.x - Rounding, b.y - Rounding), Rounding, 0.f, IM_PI / 2.f);
		DrawList->PathArcTo(ImVec2(a.x + Rounding, b.y - Rounding), Rounding, IM_PI / 2.f, IM_PI);
	}
	DrawList->PathFillConvex(color);
}

//两个颜色之间的渐变
void ImDrawList::AddRectFilledMultiColorRounded(const ImVec2& p_min, const ImVec2& p_max, ImU32 bg_color, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left, float rounding, ImDrawFlags rounding_corners)
{
	rounding = ImMin(rounding, ImFabs(p_max.x - p_min.x) * (((rounding_corners & ImDrawCornerFlags_Top) == ImDrawCornerFlags_Top) || ((rounding_corners & ImDrawCornerFlags_Bot) == ImDrawCornerFlags_Bot) ? 0.5f : 1.0f) - 1.0f);
	rounding = ImMin(rounding, ImFabs(p_max.y - p_min.y) * (((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f) - 1.0f);

	if (rounding_corners == 0)
		return;
	else
	{
		const float rounding_tl = (rounding_corners & ImDrawCornerFlags_TopLeft) ? rounding : 0.0f;
		const float rounding_tr = (rounding_corners & ImDrawCornerFlags_TopRight) ? rounding : 0.0f;
		const float rounding_br = (rounding_corners & ImDrawCornerFlags_BotRight) ? rounding : 0.0f;
		const float rounding_bl = (rounding_corners & ImDrawCornerFlags_BotLeft) ? rounding : 0.0f;

		const ImVec2 uv = _Data->TexUvWhitePixel;
		PrimReserve(6, 4);
		PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2));
		PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx + 3));
		PrimWriteVtx(p_min, uv, col_upr_left);
		PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
		PrimWriteVtx(p_max, uv, col_bot_right);
		PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);

		PathLineTo(p_min);
		PathArcTo(ImVec2(p_min.x + rounding_tl, p_min.y + rounding_tl), rounding_tl, 4.820f, 3.100f);
		PathFillConvex(bg_color);

		PathLineTo(ImVec2(p_max.x, p_min.y));
		PathArcTo(ImVec2(p_max.x - rounding_tr, p_min.y + rounding_tr), rounding_tr, 6.3400f, 4.620f);
		PathFillConvex(bg_color);

		PathLineTo(ImVec2(p_max.x, p_max.y));
		PathArcTo(ImVec2(p_max.x - rounding_br, p_max.y - rounding_br), rounding_br, 7.960f, 6.240f);
		PathFillConvex(bg_color);

		PathLineTo(ImVec2(p_min.x, p_max.y));
		PathArcTo(ImVec2(p_min.x + rounding_bl, p_max.y - rounding_bl), rounding_bl, 9.5f, 7.770f);
		PathFillConvex(bg_color);

	}
}


void RectangleFilledGraident(ImDrawList* DrawList, float x, float y, float width, float height, ImColor BgColor, ImColor TopColor, ImColor BotColor)
{
	ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All;
	ImVec2 a = ImVec2(x, y);
	ImVec2 b = ImVec2(x + width, y + height);

	float Rounding = RandomPara<float>(0.0f, 3.0f);

	Rounding = ImMin<float>(Rounding, fabsf(width) * (((rounding_corners & ImDrawCornerFlags_Top) == ImDrawCornerFlags_Top) || ((rounding_corners & ImDrawCornerFlags_Bot) == ImDrawCornerFlags_Bot) ? 0.5f : 1.0f) - 1.0f);
	Rounding = ImMin<float>(Rounding, fabsf(height) * (((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f) - 1.0f);

	DrawList->AddRectFilledMultiColorRounded(a, b, BgColor, TopColor, TopColor, BotColor, BotColor, Rounding, rounding_corners);
}



//射线检测
CGameTrace TraceShape(Vector3& vecStart, Vector3& vecEnd, uintptr_t pSkipEntity, const uint64_t nMask)
{
	const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));
	using TTaceRay = bool (*)(void*, CTraceRay*, float*, float*, CTraceFilter*, CGameTrace*);
	using Filter = void (*)(CTraceFilter*, void*, uint64_t, uint8_t, uint16_t);

	float Start[3] = { vecStart.x,vecStart.y,vecStart.z };
	float End[3] = { vecEnd.x,vecEnd.y,vecEnd.z };

	TTaceRay pfnTraceShape = (TTaceRay)(client + 0x6F26F0);
	Filter pfnCreateFilter = (Filter)(client + 0x2011F0);
	void* pTraceManager = *(void**)(client + 0x1BBB7D8);

	CTraceRay pTraceRay{};
	CTraceFilter pFilter{};
	CGameTrace pGameTrace{};
	pfnCreateFilter(&pFilter, (void*)pSkipEntity, nMask, 4, 15);
	pfnTraceShape(pTraceManager, &pTraceRay, Start, End, &pFilter, &pGameTrace);
	return pGameTrace;
}



ImColor Mix(ImColor Col_1, ImColor Col_2, float t)
{
	ImColor Col;
	Col.Value.x = t * Col_1.Value.x + (1 - t) * Col_2.Value.x;
	Col.Value.y = t * Col_1.Value.y + (1 - t) * Col_2.Value.y;
	Col.Value.z = t * Col_1.Value.z + (1 - t) * Col_2.Value.z;
	Col.Value.w = Col_1.Value.w;
	return Col;
}

ImColor FirstStageColor = ImColor(0, 255, 0, 255);

ImColor SecondStageColor = ImColor(255, 232, 0, 255);

ImColor ThirdStageColor = ImColor(255, 39, 0, 255);

ImColor BackupHealthColor = ImColor(255, 255, 255, 220);

ImColor FrameColor = ImColor(45, 45, 45, 220);

ImColor BackGroundColor = ImColor(0, 0, 0, 255);

ImColor AmmoColor = ImColor(255, 255, 0, 255);

ImColor ArmorColor = ImColor(0, 128, 255, 255);
ImColor ArmorWithHelmetColor = ImColor(255, 0, 255, 255);


auto InRange = [&](float value, float min, float max) -> bool
	{
		return value > min && value <= max;
	};





void UpdateAimbotToggle() {
	bool currentKeyState = (GetAsyncKeyState(cs::visuals::aimbotHotkey) & 0x8000);

	if (currentKeyState && !cs::visuals::prevHotkeyState) {
		cs::visuals::aimbotToggle = !cs::visuals::aimbotToggle;
	}

	cs::visuals::prevHotkeyState = currentKeyState;
}






//道具投掷
void GHelperbool(const Vector3& worldPosition, std::string Name, float* viewMatrix, int screenWidth, int screenHeight)
{

	Vector3 g;
	Vector3 screenPos;
	if (WorldToScreen(worldPosition, screenPos, viewMatrix, screenWidth, screenHeight)) {
		auto drawList = ImGui::GetForegroundDrawList();

		float radius = 8.0f;
		ImVec2 center(screenPos.x, screenPos.y);

		// 圆形底色
		ImU32 fillColor = IM_COL32(0, 122, 255, 180);     // 半透明蓝色
		ImU32 borderColor = IM_COL32(255, 255, 255, 255); // 白色描边

		drawList->AddCircleFilled(center, radius, fillColor, 12);
		drawList->AddCircle(center, radius, borderColor, 12, 1.5f);

		// 半透明标签背景
		std::string labelText = Name;
		ImVec2 textSize = ImGui::CalcTextSize(labelText.c_str());
		ImVec2 textPos = ImVec2(center.x - textSize.x / 2, center.y - radius - textSize.y - 4);
		ImVec2 bgMin = ImVec2(textPos.x - 4, textPos.y - 2);
		ImVec2 bgMax = ImVec2(textPos.x + textSize.x + 4, textPos.y + textSize.y + 2);


		drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 160), 4.0f);
		drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), labelText.c_str());
	}

}


void AngleHelper(const Vector3& EyePosition, QAngle angles, std::string Name, float* viewMatrix, int screenWidth, int screenHeight)
{

	Vector3 point = AngleToWorldPosition(EyePosition, angles, 700.0f);

	Vector3 screenPos;
	if (WorldToScreen(point, screenPos, viewMatrix, screenWidth, screenHeight)) {
		auto drawList = ImGui::GetForegroundDrawList();

		float radius = 8.0f;
		ImVec2 center(screenPos.x, screenPos.y);

		// 圆形底色
		ImU32 fillColor = IM_COL32(0, 122, 255, 180);     // 半透明蓝色
		ImU32 borderColor = IM_COL32(255, 255, 255, 255); // 白色描边

		drawList->AddCircleFilled(center, radius, fillColor, 12);
		drawList->AddCircle(center, radius, borderColor, 12, 1.5f);

		// 半透明标签背景
		std::string labelText = Name;
		ImVec2 textSize = ImGui::CalcTextSize(labelText.c_str());
		ImVec2 textPos = ImVec2(center.x - textSize.x / 2, center.y - radius - textSize.y - 4);
		ImVec2 bgMin = ImVec2(textPos.x - 4, textPos.y - 2);
		ImVec2 bgMax = ImVec2(textPos.x + textSize.x + 4, textPos.y + textSize.y + 2);


		drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 160), 4.0f);
		drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), labelText.c_str());
	}

}


void draw_esp() {
	const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));

	auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);

	if (!local_ctrl)
	{
		return;
	}
	auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
	if (local_hpawn == 0xFFFFFFFF)
	{
		return;
	}

	auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);
	if (!localpawn)
	{
		return;
	}
	auto Matrix = reinterpret_cast<float*>(client + cs2_dumper::offsets::client_dll::dwViewMatrix);
	if (!Matrix)
	{
		return;
	}
	auto health = *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
	auto local_eyepos_op_vec = GetEyePos(localpawn);//本地玩家三维视角
	if (!local_eyepos_op_vec.has_value())
		return;
	auto local_eyepos = local_eyepos_op_vec.value();//本地玩家三维视角
	//本地玩家武器
	auto local_weaponServices = *reinterpret_cast<uintptr_t*>(localpawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_pWeaponServices);
	if (!local_weaponServices) {
		return;
	}
	// 获取当前武器句柄（使用与偏移匹配的类型）
	uint32_t activeWeaponHandle = *reinterpret_cast<uint32_t*>(local_weaponServices + cs2_dumper::schemas::client_dll::CPlayer_WeaponServices::m_hActiveWeapon);
	// 检查无效句柄（32位无效值）
	if (activeWeaponHandle == 0xFFFFFFFF) {
		return;
	}
	// 解析武器实体地址
	uintptr_t weaponBase = GetBaseEntityFromHandle(activeWeaponHandle, client);
	if (!weaponBase) {
		return;
	}
	//武器ID
	auto local_weaponID = *reinterpret_cast<uint16_t*>(weaponBase + cs2_dumper::schemas::client_dll::C_EconEntity::m_AttributeManager + cs2_dumper::schemas::client_dll::C_AttributeContainer::m_Item + cs2_dumper::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex);
	std::string local_weaponName = GetWeaponName(int(local_weaponID));
	std::vector<NadePoint> nade;
	//本地玩家坐标
	auto local_Origin = *reinterpret_cast<Vector3*>(localpawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
	if (local_Origin.IsVectorEmpty())
	{
		return;
	}
	const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	if (cs::visuals::help)
	{
		if (cs::visuals::loadnade) {
			LoadNadeList("D:\\CS_hacking\\cs_hacking\\cs_hacking\\hacking\\nadepoints.json", nade);
			for (const auto& nade : nade) {
				if (local_weaponName == nade.GrenadeName)
				{
					GHelperbool(nade.POS, nade.firstname.c_str(), Matrix, screenWidth, screenHeight);
					//   GHelperbool(nade.POSs1, nade.name.c_str(), viewMatrix, screenWidth, screenHeight);
					AngleHelper(nade.Eyes, nade.Angle, nade.lastname.c_str(), Matrix, screenWidth, screenHeight);

				}
			}
		}
		bool keyPressed_nade = (GetAsyncKeyState(cs::visuals::aimbotHotkey_nade) & 0x1);
		if (cs::visuals::RecordHelper && keyPressed_nade){
			QAngle Angles = *reinterpret_cast<QAngle*>(client + cs2_dumper::offsets::client_dll::dwViewAngles);
			NadePoint newPoint = {
				u8"默认", // 起始点位名称
				u8"默认",  // 终点位名称
				local_weaponName,
				local_Origin,                       // 站位坐标
				Angles,     // 角度
				local_eyepos                                // 摄像机坐标
			};

			// 准备保存点位的容器
			std::vector<NadePoint> nadePoints;

			// 尝试读取旧数据
			std::ifstream inFile("D:\\CS_hacking\\cs_hacking\\cs_hacking\\hacking\\nadepoints.json");
			if (inFile.is_open()) {
				try {
					nlohmann::json oldJson;
					inFile >> oldJson;
					inFile.close();

					nadePoints = oldJson.get<std::vector<NadePoint>>();
				}
				catch (...) {
					inFile.close();
				}
			}

			// 添加新数据
			nadePoints.push_back(newPoint);

			// 保存到文件
			nlohmann::json j = nadePoints;
			std::ofstream outFile("D:\\CS_hacking\\cs_hacking\\cs_hacking\\hacking\\nadepoints.json");
			outFile << j.dump(2); // 
			outFile.close();
		}
	}
	auto localteam = *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
	if (!localteam)
	{
		return;
	}
	Vector3 Local_Head_Pos = BonePos(localpawn, 6);
	
	//int Index;
	int closest = 999999;
	Vector3 bestAimTarget{};
	//自瞄热键单击|长按切换
	bool keyPressed = (GetAsyncKeyState(cs::visuals::aimbotHotkey) & 0x1);
	bool keyHeld = (GetAsyncKeyState(cs::visuals::aimbotHotkey) & 0x8000);
	//记录点位
	
	const char* boneNames[] = {
	u8"pelvis",         // pelvis
	u8"spine_2",        // spine_2
	u8"spine_1",        // spine_1
	u8"neck_0",         // neck_0
	u8"head",         // head
	u8"arm_upper_L",       // arm_upper_L
	u8"arm_lower_L",       // arm_lower_L
	u8"hand_L",         // hand_L
	u8"arm_upper_R",       // arm_upper_R
	u8"arm_lower_R",       // arm_lower_R
	u8"hand_R",         // hand_R
	u8"leg_upper_L",       // leg_upper_L
	u8"leg_lower_L",       // leg_lower_L
	u8"ankle_L",       // ankle_L
	u8"leg_upper_R",       // leg_upper_R
	u8"leg_lower_R",       // leg_lower_R
	u8"ankle_R",       // ankle_R
	};
	const std::unordered_map<std::string, int> boneNameToValue = {
	{u8"pelvis",         0},
	{u8"spine_2",        2},
	{u8"spine_1",        4},
	{u8"neck_0",         5},
	{u8"head",           6},  // head对应的值为6
	{u8"arm_upper_L",    8},
	{u8"arm_lower_L",    9},
	{u8"hand_L",        10},
	{u8"arm_upper_R",   13},
	{u8"arm_lower_R",   14},
	{u8"hand_R",        15},
	{u8"leg_upper_L",   22},
	{u8"leg_lower_L",   23},
	{u8"ankle_L",       24},
	{u8"leg_upper_R",   25},
	{u8"leg_lower_R",   26},
	{u8"ankle_R",       27},
	};
	for (int i = 0; i < 64; i++) {
		auto player_co = GetBaseEntity(i, client);
		if (!player_co)
			continue;
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
		
		auto player_health = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
		if (player_health <= 0 || player_health > 100)
			continue;
		auto name = *reinterpret_cast<char**>(player_co + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName);
		
		std::string name_str(name);
		
		
		//金钱
		auto moneyServices = *reinterpret_cast<uintptr_t*>(player_co + cs2_dumper::schemas::client_dll::CCSPlayerController::m_pInGameMoneyServices);
		if (!moneyServices) {
			continue;
		}
		auto money = *reinterpret_cast<int*>(moneyServices + cs2_dumper::schemas::client_dll::CCSPlayerController_InGameMoneyServices::m_iAccount);
		std::string money_str = Format("%s:%i", name_str, money);
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
		// 主武器当前弹夹弹药
		auto player_iAmmo1 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_iClip1);
		//主武器弹夹弹药
		//auto player_iMaxAmmo1 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::CBasePlayerWeaponVData::m_iMaxClip1);
		//主武器最大弹药
		auto player_pReserveAmmo1 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_pReserveAmmo);
		
		// 副武器当前弹夹弹药
		auto player_iAmmo2 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_iClip2);
		
		//副武器最大弹药
		auto player_pReserveAmmo2 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_pReserveAmmo + 4);
		//武器ID
		auto player_weaponID = *reinterpret_cast<uint16_t*>(weaponBase + cs2_dumper::schemas::client_dll::C_EconEntity::m_AttributeManager + cs2_dumper::schemas::client_dll::C_AttributeContainer::m_Item + cs2_dumper::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex);


		//护甲值
		auto player_armor = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_ArmorValue);
		//是否头盔
		auto ItemServices = *reinterpret_cast<uintptr_t*>(player_pawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_pItemServices);
		if (!ItemServices) {
			continue;
		}

		auto player_bHasHelmet = *reinterpret_cast<bool*>(ItemServices + cs2_dumper::schemas::client_dll::CCSPlayer_ItemServices::m_bHasHelmet);
		//是否有拆弹器
		auto player_bHasDefuser = *reinterpret_cast<bool*>(ItemServices + cs2_dumper::schemas::client_dll::CCSPlayer_ItemServices::m_bHasDefuser);
		
		//玩家三维坐标
		auto player_Origin = *reinterpret_cast<Vector3*>(player_pawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
		if (player_Origin.IsVectorEmpty())
			continue;
		auto player_eyepos_op_vec = GetEyePos(player_pawn);

		if (!player_eyepos_op_vec.has_value())
			continue;
		auto player_eyepos = player_eyepos_op_vec.value();
		Vector3 Player_Head_Pos = BonePos(player_pawn, 6);


		Vector3 head_pos_2d{};
		Vector3 abs_pos_2d{};
		static const float w = ImGui::GetIO().DisplaySize.x;
		static const float h = ImGui::GetIO().DisplaySize.y;
		if (!WorldToScreen(player_Origin, abs_pos_2d, Matrix, w, h))
		{
			continue;
		}

		if (!WorldToScreen(player_eyepos, head_pos_2d, Matrix, w, h))
		{
			continue;
		}

		//判断是否发现敌人
		CGameTrace pGameTrace = TraceShape(Local_Head_Pos, Player_Head_Pos, localpawn, 0xC3003);
		bool bVisible = pGameTrace.pHitEntity == (void*)player_pawn;




		//头顶圆圈
		Vector3 head_pos{};
		Vector3 neck_pos{};
		Vector3 aimbot_head_pos{};
		Vector3 aimbot_pos{};
		if (!WorldToScreen(BonePos(player_pawn, Bone_Base::BoneIndex::head), head_pos, Matrix, w, h)) {
			continue;
		}
		if (!WorldToScreen(BonePos(player_pawn, Bone_Base::BoneIndex::neck_0), neck_pos, Matrix, w, h)) {
			continue;
		};
		float Radius = abs(head_pos.y - neck_pos.y) + 3;




		const float height{ ::abs(head_pos_2d.y - abs_pos_2d.y) * 1.25f };
		const float width{ height / 2.f };
		const float x = head_pos_2d.x - (width / 2.f);//左上角x
		const float y = head_pos_2d.y - (width / 2.5f);//左上角y
		//距离计算
		int distance = static_cast<int>(sqrtf(powf(player_Origin.x - local_Origin.x, 2) + powf(player_Origin.y - local_Origin.y, 2) + powf(player_Origin.z - local_Origin.z, 2)) / 100);
		std::string dis_str = Format(u8"%i米", distance);

		//血条


		ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

		float Proportion = static_cast<float>(player_health) / 100;
		float Height = height * Proportion;
		ImColor Color;
		float Color_Lerp_t = pow(Proportion, 2.5);
		if (InRange(Proportion, 0.5, 1))
			Color = Mix(FirstStageColor, SecondStageColor, Color_Lerp_t * 3 - 1);
		else
			Color = Mix(SecondStageColor, ThirdStageColor, Color_Lerp_t * 4);



		
		//填充方框
		if (cs::visuals::FilledBox)
		{
			if (cs::visuals::FilledVisBox) {
				// visCheck from @KeysIsCool：判断实体是否被本地玩家发现

				if (bVisible) {
					DrawFilledRoundedBox(DrawList, x, y, width, height, ImColor(cs::visuals::PathFillConvex));
				}
				else {
					DrawFilledRoundedBox(DrawList, x, y, width, height, ImColor(cs::visuals::FillConvex));
				}
			}
			else {
				if (cs::visuals::boxMultiColor)
				{
					RectangleFilledGraident(DrawList, x, y, width, height, ImColor(cs::visuals::boxColor), ImColor(cs::visuals::FillConvex), ImColor(cs::visuals::FillConvex2));
				}
				else
				{
					DrawFilledRoundedBox(DrawList, x, y, width, height, ImColor(cs::visuals::FillConvex));
				}
			}
		}
		
		if (cs::visuals::box)
		{
			if (cs::visuals::SelectedBoxType == 0)
			{
				if (cs::visuals::OutLine)
					DrawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, RandomPara<float>(0.0f, 3.0f), 0, 3);

				if (bVisible)
				{
					DrawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), ImColor(cs::visuals::PathFillConvex), RandomPara<float>(0.0f, 3.0f), 0, 1.3);
				}
				else {
					DrawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), ImColor(cs::visuals::boxColor), RandomPara<float>(0.0f, 3.0f), 0, 1.3);
				}
			}
			else
			{
				DrawList->AddLine(ImVec2(x, y), ImVec2(x +  width * 0.25f, y), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
				DrawList->AddLine(ImVec2(x, y), ImVec2(x, y + height * 0.25f), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
				DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width -  width * 0.25f, y + height), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
				DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width, y + height - height * 0.25f), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
				DrawList->AddLine(ImVec2(x, y + height), ImVec2(x +  width * 0.25f, y + height), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
				DrawList->AddLine(ImVec2(x, y + height), ImVec2(x, y + height - height * 0.25f), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
				DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width -  width * 0.25f, y), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
				DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + height * 0.25f), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);

				// Main Box Lines
				if (bVisible)
				{
					DrawList->AddLine(ImVec2(x, y), ImVec2(x +  width * 0.25f, y), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x, y), ImVec2(x, y + height * 0.25f), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width -  width * 0.25f, y), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + height * 0.25f), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x, y + height), ImVec2(x +  width * 0.25f, y + height), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x, y + height), ImVec2(x, y + height - height * 0.25f), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width -  width * 0.25f, y + height), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width, y + height - height * 0.25f), ImColor(cs::visuals::PathFillConvex), 1.3f);
				}
				else {
					DrawList->AddLine(ImVec2(x, y), ImVec2(x +  width * 0.25f, y), ImColor(cs::visuals::boxColor), 1.3f);
					DrawList->AddLine(ImVec2(x, y), ImVec2(x, y + height * 0.25f), ImColor(cs::visuals::boxColor), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width -  width * 0.25f, y), ImColor(cs::visuals::boxColor), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + height * 0.25f), ImColor(cs::visuals::boxColor), 1.3f);
					DrawList->AddLine(ImVec2(x, y + height), ImVec2(x +  width * 0.25f, y + height), ImColor(cs::visuals::boxColor), 1.3f);
					DrawList->AddLine(ImVec2(x, y + height), ImVec2(x, y + height - height * 0.25f), ImColor(cs::visuals::boxColor), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width -  width * 0.25f, y + height), ImColor(cs::visuals::boxColor), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width, y + height - height * 0.25f), ImColor(cs::visuals::boxColor), 1.3f);
				}
			}
		}
		//骨骼
		if (cs::visuals::bone)
		{
			if (bVisible) {
				Bone_Start(player_pawn, ImColor(cs::visuals::boneVisColor), Matrix);
			}
			else {
				Bone_Start(player_pawn, ImColor(cs::visuals::boneColor), Matrix);
			}
			
		}

		//护甲
		if (cs::visuals::armor) {
			if (player_armor > 0) {

				float armorProportion = static_cast<float>(player_armor) / 100;
				float armorHeight = height * armorProportion;
				ImColor armorColor;

				DrawList->AddRectFilled({ x - 4, y },
					{ x, abs_pos_2d.y },
					BackGroundColor, 5, 15);
				if (player_bHasHelmet)
					armorColor = ArmorWithHelmetColor;
				else
					armorColor = ArmorColor;
				DrawList->AddRectFilled({ x - 4, abs_pos_2d.y - armorHeight },
					{ x,abs_pos_2d.y },
					armorColor, 0);

				DrawList->AddRect({ x - 4, y },
					{ x,abs_pos_2d.y },
					FrameColor, 0, 15, 1);
				
				if (cs::visuals::armorNum)
				{
					if (player_health < 100)
					{
						std::string armor_str = Format("%d", player_armor);
						ImVec2 Pos = { x-4,abs_pos_2d.y - armorHeight };
						DrawList->AddText(Pos, ImColor(cs::visuals::healthColor), armor_str.c_str());
					}
				}
			}
		}
		//弹药条
		if (cs::visuals::AmmoBar) {
			if (player_iAmmo1 >= 0) {
				float AmmoBarProportion = 0.0f;
				if (player_pReserveAmmo1 >= player_iAmmo1) {
					AmmoBarProportion = static_cast<float>(player_iAmmo1) / player_pReserveAmmo1;
				}
				else {
					AmmoBarProportion = 1.0f;
				}
				float AmmoBarWidth = width * AmmoBarProportion;
				ImColor AmmoBarColor;

				DrawList->AddRectFilled({ x, abs_pos_2d.y },
					{ x, abs_pos_2d.y + 4 },
					BackGroundColor, 5, 15);
				DrawList->AddRectFilled({ x, abs_pos_2d.y },
					{ x + AmmoBarWidth, abs_pos_2d.y + 4 },
					AmmoColor, 0);

				DrawList->AddRect({ x, abs_pos_2d.y },
					{ x, abs_pos_2d.y + 4 },
					FrameColor, 0, 15, 1);

				if (cs::visuals::AmmoNum)
				{

					std::string Ammo_str = Format("%d|%d", player_iAmmo1, player_pReserveAmmo1);
					ImVec2 Pos = { x + AmmoBarWidth,abs_pos_2d.y + 4 };
					DrawList->AddText(Pos, ImColor(cs::visuals::healthColor), Ammo_str.c_str());
				}
			}
		}
		//血条
		if (cs::visuals::healthbar) {
			std::string health_str = Format("%d", player_health);
			if (cs::visuals::armor) {
				DrawList->AddRectFilled({ x - 8, y },
					{ x - 4, abs_pos_2d.y },
					BackGroundColor, 5, 15);
				DrawList->AddRectFilled({ x - 8, abs_pos_2d.y - Height },
					{ x - 4,abs_pos_2d.y },
					Color, 0);

				DrawList->AddRect({ x - 8, y },
					{ x - 4,abs_pos_2d.y },
					FrameColor, 0, 15, 1);
				if (cs::visuals::health)
				{
					ImVec2 Pos = { x - 12,abs_pos_2d.y - Height };
					DrawList->AddText(Pos, ImColor(cs::visuals::healthColor), health_str.c_str());

				}
			}
			else {
				DrawList->AddRectFilled({ x - 4, y },
					{ x, abs_pos_2d.y },
					BackGroundColor, 5, 15);
				DrawList->AddRectFilled({ x - 4, abs_pos_2d.y - Height },
					{ x,abs_pos_2d.y },
					Color, 0);

				DrawList->AddRect({ x - 4, y },
					{ x,abs_pos_2d.y },
					FrameColor, 0, 15, 1);
				if (cs::visuals::health)
				{
					ImVec2 Pos = { x - 8,abs_pos_2d.y - Height };
					DrawList->AddText(Pos, ImColor(cs::visuals::healthColor), health_str.c_str());

				}
			}
		}
		//头部
		if (cs::visuals::HeadCircle) {
			if (bVisible) {
				DrawList->AddCircle(ImVec2(head_pos.x, head_pos.y), Radius, ImColor(cs::visuals::boneVisColor));
			}
			else {
				DrawList->AddCircle(ImVec2(head_pos.x, head_pos.y), Radius, ImColor(cs::visuals::boneColor));
			}
		}
		//距离
		if (cs::visuals::distance) {
			DrawList->AddText(ImVec2(head_pos_2d.x + (width / 2.f), y + 2), ImColor(cs::visuals::distanceColor), dis_str.c_str());
		}
		//武器名称
		if (cs::visuals::weapon) {
			
			std::string weaponName = GetWeaponName(int(player_weaponID));
			//DrawList->AddText(ImVec2(head_pos_2d.x - 8, y - 10 ), ImColor(cs::visuals::weaponColor), weaponName.c_str());
			std::string weaponIcon = GetWeaponIcon(int(player_weaponID));
			ImGui::PushFont(g_font_icon);
			DrawList->AddText(g_font_icon, 16.0f, ImVec2(head_pos_2d.x - 14, y - 30), ImColor(cs::visuals::weaponColor), weaponIcon.c_str());
			if (player_bHasDefuser) {
				DrawList->AddText(g_font_icon, 16.0f, ImVec2(head_pos_2d.x + (width / 2.f), y + height - 10), ImColor(cs::visuals::weaponColor), "r");
			}
			
			ImGui::PopFont();  // 恢复默认字体
			}
		//自瞄
		if (cs::visuals::aimbot && WorldToScreen(BonePos(player_pawn, Bone_Base::BoneIndex::head), aimbot_head_pos, Matrix, w, h) && bVisible) {
			drawFovCircle(cs::visuals::FOV);
			const char* selectedBoneName = boneNames[cs::visuals::current_item];
			int selectedBoneValue = -1;
			auto it = boneNameToValue.find(selectedBoneName);
			if (it != boneNameToValue.end()) {
				selectedBoneValue = it->second;
			}
			WorldToScreen(BonePos(player_pawn, selectedBoneValue), aimbot_pos, Matrix, w, h);
			float dist = getDistanceToCenter(aimbot_pos);
			//DrawList->AddText(ImVec2(head_pos_2d.x - 12, y - 20), ImColor(cs::visuals::weaponColor), distStr.c_str());
			if (cs::visuals::toggleMode) {
				if (keyPressed) {
					cs::visuals::aimbotActive = !cs::visuals::aimbotActive;
				}
			}
			else {
				cs::visuals::aimbotActive = keyHeld;
			}
			if (dist < cs::visuals::FOV && dist < closest  && cs::visuals::aimbotActive) {
				closest = dist;
				bestAimTarget = aimbot_pos;
				startAimbot(bestAimTarget, cs::visuals::Smoothness);
			}
		}
		//自瞄扳机
		if (cs::visuals::autoaim && WorldToScreen(BonePos(player_pawn, Bone_Base::BoneIndex::head), aimbot_head_pos, Matrix, w, h)) {
			drawFovCircle(10.0f);
			float dist = getDistanceToCenter(aimbot_head_pos);
			std::string distStr = std::to_string(dist);
			//DrawList->AddText(ImVec2(head_pos_2d.x - 12, y - 20), ImColor(cs::visuals::weaponColor), distStr.c_str());
			
			if (dist < 10.0f && dist < closest ) {
				closest = dist;
				bestAimTarget = aimbot_head_pos;
				startAimbot(bestAimTarget, 0.4f);
			}
			if (closest < 10.0f && bVisible) {
				mouse_left_click();
			}
		}
	}
}

void test() {
	const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));

	auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);

	if (!local_ctrl)
	{
		return;
	}
	auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
	if (local_hpawn == 0xFFFFFFFF)
	{
		return;
	}

	auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);
	if (!localpawn)
	{
		return;
	}
	auto Matrix = reinterpret_cast<float*>(client + cs2_dumper::offsets::client_dll::dwViewMatrix);
	if (!Matrix)
	{
		return;
	}
	auto health = *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
	if (health == 0) {
		for (int i = 0; i < 64; i++) {
			auto player_co = GetBaseEntity(i, client);
			if (!player_co)
				continue;
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
			if (cs::visuals::team_CT) {
				if (player_team == 3) {
					continue;
				}
			}
			if (cs::visuals::team_T) {
				if (player_team == 2) {
					continue;
				}
			}

			auto player_health = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
			if (player_health <= 0 || player_health > 100)
				continue;

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
			// 主武器当前弹夹弹药
			auto player_iAmmo1 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_iClip1);
			//主武器弹夹弹药
			//auto player_iMaxAmmo1 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::CBasePlayerWeaponVData::m_iMaxClip1);
			//主武器最大弹药
			auto player_pReserveAmmo1 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_pReserveAmmo);

			// 副武器当前弹夹弹药
			auto player_iAmmo2 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_iClip2);

			//副武器最大弹药
			auto player_pReserveAmmo2 = *reinterpret_cast<int*>(weaponBase + cs2_dumper::schemas::client_dll::C_BasePlayerWeapon::m_pReserveAmmo + 4);
			//武器ID
			auto player_weaponID = *reinterpret_cast<uint16_t*>(weaponBase + cs2_dumper::schemas::client_dll::C_EconEntity::m_AttributeManager + cs2_dumper::schemas::client_dll::C_AttributeContainer::m_Item + cs2_dumper::schemas::client_dll::C_EconItemView::m_iItemDefinitionIndex);


			//护甲值
			auto player_armor = *reinterpret_cast<int*>(player_pawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_ArmorValue);
			//是否头盔
			auto ItemServices = *reinterpret_cast<uintptr_t*>(player_pawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_pItemServices);
			if (!ItemServices) {
				continue;
			}

			auto player_bHasHelmet = *reinterpret_cast<bool*>(ItemServices + cs2_dumper::schemas::client_dll::CCSPlayer_ItemServices::m_bHasHelmet);
			auto player_bHasDefuser = *reinterpret_cast<bool*>(ItemServices + cs2_dumper::schemas::client_dll::CCSPlayer_ItemServices::m_bHasDefuser);

			//玩家三维坐标
			auto player_Origin = *reinterpret_cast<Vector3*>(player_pawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
			if (player_Origin.IsVectorEmpty())
				continue;
			auto player_eyepos_op_vec = GetEyePos(player_pawn);

			if (!player_eyepos_op_vec.has_value())
				continue;
			auto player_eyepos = player_eyepos_op_vec.value();
			Vector3 Player_Head_Pos = BonePos(player_pawn, 6);


			Vector3 head_pos_2d{};
			Vector3 abs_pos_2d{};
			static const float w = ImGui::GetIO().DisplaySize.x;
			static const float h = ImGui::GetIO().DisplaySize.y;
			if (!WorldToScreen(player_Origin, abs_pos_2d, Matrix, w, h))
			{
				continue;
			}

			if (!WorldToScreen(player_eyepos, head_pos_2d, Matrix, w, h))
			{
				continue;
			}






			//头顶圆圈
			Vector3 head_pos{};
			Vector3 neck_pos{};
			Vector3 aimbot_head_pos{};
			Vector3 aimbot_pos{};
			if (!WorldToScreen(BonePos(player_pawn, Bone_Base::BoneIndex::head), head_pos, Matrix, w, h)) {
				continue;
			}
			if (!WorldToScreen(BonePos(player_pawn, Bone_Base::BoneIndex::neck_0), neck_pos, Matrix, w, h)) {
				continue;
			};
			float Radius = abs(head_pos.y - neck_pos.y) + 3;




			const float height{ ::abs(head_pos_2d.y - abs_pos_2d.y) * 1.25f };
			const float width{ height / 2.f };
			const float x = head_pos_2d.x - (width / 2.f);//左上角x
			const float y = head_pos_2d.y - (width / 2.5f);//左上角y


			//血条


			ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

			float Proportion = static_cast<float>(player_health) / 100;
			float Height = height * Proportion;
			ImColor Color;
			float Color_Lerp_t = pow(Proportion, 2.5);
			if (InRange(Proportion, 0.5, 1))
				Color = Mix(FirstStageColor, SecondStageColor, Color_Lerp_t * 3 - 1);
			else
				Color = Mix(SecondStageColor, ThirdStageColor, Color_Lerp_t * 4);




			//填充方框
			if (cs::visuals::FilledBox)
			{
				if (cs::visuals::FilledVisBox) {
					// visCheck from @KeysIsCool：判断实体是否被本地玩家发现
					DrawFilledRoundedBox(DrawList, x, y, width, height, ImColor(cs::visuals::PathFillConvex));


				}
				else {
					if (cs::visuals::boxMultiColor)
					{
						RectangleFilledGraident(DrawList, x, y, width, height, ImColor(cs::visuals::boxColor), ImColor(cs::visuals::FillConvex), ImColor(cs::visuals::FillConvex2));
					}
					else
					{
						DrawFilledRoundedBox(DrawList, x, y, width, height, ImColor(cs::visuals::FillConvex));
					}
				}
			}

			if (cs::visuals::box)
			{
				if (cs::visuals::SelectedBoxType == 0)
				{
					if (cs::visuals::OutLine)
						DrawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, RandomPara<float>(0.0f, 3.0f), 0, 3);

					DrawList->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), ImColor(cs::visuals::PathFillConvex), RandomPara<float>(0.0f, 3.0f), 0, 1.3);

				}
				else
				{
					DrawList->AddLine(ImVec2(x, y), ImVec2(x + width * 0.25f, y), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
					DrawList->AddLine(ImVec2(x, y), ImVec2(x, y + height * 0.25f), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
					DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width - width * 0.25f, y + height), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
					DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width, y + height - height * 0.25f), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
					DrawList->AddLine(ImVec2(x, y + height), ImVec2(x + width * 0.25f, y + height), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
					DrawList->AddLine(ImVec2(x, y + height), ImVec2(x, y + height - height * 0.25f), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
					DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width - width * 0.25f, y), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);
					DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + height * 0.25f), ImColor(cs::visuals::boxColor) & IM_COL32_A_MASK, 3);

					// Main Box Lines

					DrawList->AddLine(ImVec2(x, y), ImVec2(x + width * 0.25f, y), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x, y), ImVec2(x, y + height * 0.25f), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width - width * 0.25f, y), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y), ImVec2(x + width, y + height * 0.25f), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x, y + height), ImVec2(x + width * 0.25f, y + height), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x, y + height), ImVec2(x, y + height - height * 0.25f), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width - width * 0.25f, y + height), ImColor(cs::visuals::PathFillConvex), 1.3f);
					DrawList->AddLine(ImVec2(x + width, y + height), ImVec2(x + width, y + height - height * 0.25f), ImColor(cs::visuals::PathFillConvex), 1.3f);


				}
			}
			//骨骼
			if (cs::visuals::bone)
			{

				Bone_Start(player_pawn, ImColor(cs::visuals::boneVisColor), Matrix);


			}

			//护甲
			if (cs::visuals::armor) {
				if (player_armor > 0) {

					float armorProportion = static_cast<float>(player_armor) / 100;
					float armorHeight = height * armorProportion;
					ImColor armorColor;

					DrawList->AddRectFilled({ x - 4, y },
						{ x, abs_pos_2d.y },
						BackGroundColor, 5, 15);
					if (player_bHasHelmet)
						armorColor = ArmorWithHelmetColor;
					else
						armorColor = ArmorColor;
					DrawList->AddRectFilled({ x - 4, abs_pos_2d.y - armorHeight },
						{ x,abs_pos_2d.y },
						armorColor, 0);

					DrawList->AddRect({ x - 4, y },
						{ x,abs_pos_2d.y },
						FrameColor, 0, 15, 1);

					if (cs::visuals::armorNum)
					{
						if (player_health < 100)
						{
							std::string armor_str = Format("%d", player_armor);
							ImVec2 Pos = { x - 4,abs_pos_2d.y - armorHeight };
							DrawList->AddText(Pos, ImColor(cs::visuals::healthColor), armor_str.c_str());
						}
					}
				}
			}
			//弹药条
			if (cs::visuals::AmmoBar) {
				if (player_iAmmo1 >= 0) {
					float AmmoBarProportion = 0.0f;
					if (player_pReserveAmmo1 >= player_iAmmo1) {
						AmmoBarProportion = static_cast<float>(player_iAmmo1) / player_pReserveAmmo1;
					}
					else {
						AmmoBarProportion = 1.0f;
					}
					float AmmoBarWidth = width * AmmoBarProportion;
					ImColor AmmoBarColor;

					DrawList->AddRectFilled({ x, abs_pos_2d.y },
						{ x, abs_pos_2d.y + 4 },
						BackGroundColor, 5, 15);
					DrawList->AddRectFilled({ x, abs_pos_2d.y },
						{ x + AmmoBarWidth, abs_pos_2d.y + 4 },
						AmmoColor, 0);

					DrawList->AddRect({ x, abs_pos_2d.y },
						{ x, abs_pos_2d.y + 4 },
						FrameColor, 0, 15, 1);

					if (cs::visuals::AmmoNum)
					{

						std::string Ammo_str = Format("%d|%d", player_iAmmo1, player_pReserveAmmo1);
						ImVec2 Pos = { x + AmmoBarWidth,abs_pos_2d.y + 4 };
						DrawList->AddText(Pos, ImColor(cs::visuals::healthColor), Ammo_str.c_str());
					}
				}
			}
			//血条
			if (cs::visuals::healthbar) {
				std::string health_str = Format("%d", player_health);
				if (cs::visuals::armor) {
					DrawList->AddRectFilled({ x - 8, y },
						{ x - 4, abs_pos_2d.y },
						BackGroundColor, 5, 15);
					DrawList->AddRectFilled({ x - 8, abs_pos_2d.y - Height },
						{ x - 4,abs_pos_2d.y },
						Color, 0);

					DrawList->AddRect({ x - 8, y },
						{ x - 4,abs_pos_2d.y },
						FrameColor, 0, 15, 1);
					if (cs::visuals::health)
					{
						ImVec2 Pos = { x - 12,abs_pos_2d.y - Height };
						DrawList->AddText(Pos, ImColor(cs::visuals::healthColor), health_str.c_str());

					}
				}
				else {
					DrawList->AddRectFilled({ x - 4, y },
						{ x, abs_pos_2d.y },
						BackGroundColor, 5, 15);
					DrawList->AddRectFilled({ x - 4, abs_pos_2d.y - Height },
						{ x,abs_pos_2d.y },
						Color, 0);

					DrawList->AddRect({ x - 4, y },
						{ x,abs_pos_2d.y },
						FrameColor, 0, 15, 1);
					if (cs::visuals::health)
					{
						ImVec2 Pos = { x - 8,abs_pos_2d.y - Height };
						DrawList->AddText(Pos, ImColor(cs::visuals::healthColor), health_str.c_str());

					}
				}
			}
			//头部
			if (cs::visuals::HeadCircle) {

				DrawList->AddCircle(ImVec2(head_pos.x, head_pos.y), Radius, ImColor(cs::visuals::boneVisColor));

			}
			//武器名称
			if (cs::visuals::weapon) {

				std::string weaponName = GetWeaponName(int(player_weaponID));
				//DrawList->AddText(ImVec2(head_pos_2d.x - 8, y - 10 ), ImColor(cs::visuals::weaponColor), weaponName.c_str());
				std::string weaponIcon = GetWeaponIcon(int(player_weaponID));
				ImGui::PushFont(g_font_icon);
				DrawList->AddText(g_font_icon, 16.0f, ImVec2(head_pos_2d.x - 14, y - 30), ImColor(cs::visuals::weaponColor), weaponIcon.c_str());
				if (player_bHasDefuser) {
					DrawList->AddText(g_font_icon, 16.0f, ImVec2(head_pos_2d.x + (width / 2.f), y + height - 10), ImColor(cs::visuals::weaponColor), "r");
				}
				ImGui::PopFont();  // 恢复默认字体
			}
		}
	}
}

template <typename T>
class c_network_utl_vector_base
{
public:
	uint32_t m_size;
	T* m_elements;
};


void draw_money() {
	if (cs::visuals::money) {
		const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));
		ImDrawList* DrawList = ImGui::GetBackgroundDrawList();
		static const float w = ImGui::GetIO().DisplaySize.x;
		static const float h = ImGui::GetIO().DisplaySize.y;
		int count = 0;
		auto local_ctrl = *reinterpret_cast<uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwLocalPlayerController);

		if (!local_ctrl)
		{
			return;
		}
		auto local_hpawn = *reinterpret_cast<uint32_t*>(local_ctrl + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
		if (local_hpawn == 0xFFFFFFFF)
		{
			return;
		}

		auto localpawn = GetBaseEntityFromHandle(local_hpawn, client);
		if (!localpawn)
		{
			return;
		}
		auto localteam = *reinterpret_cast<int*>(localpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
		if (!localteam)
		{
			return;
		}
		for (int i = 0; i < 64; i++) {
			auto player_co = GetBaseEntity(i, client);
			if (!player_co)
				continue;
			//金钱
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
			auto moneyServices = *reinterpret_cast<uintptr_t*>(player_co + cs2_dumper::schemas::client_dll::CCSPlayerController::m_pInGameMoneyServices);
			if (!moneyServices) {
				continue;
			}
			auto name = *reinterpret_cast<char**>(player_co + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName);
			if (!type)
				continue;
			std::string name_str(name);


			auto money = *reinterpret_cast<int*>(moneyServices + cs2_dumper::schemas::client_dll::CCSPlayerController_InGameMoneyServices::m_iAccount);
			std::string money_str = Format("$%i", money);

			
			DrawList->AddText(ImVec2(w / 4 * 3 - 150, count * 15), ImColor(cs::visuals::money_color), name_str.c_str());
			DrawList->AddText(ImVec2(w / 4 * 3 - 50, count * 15), ImColor(cs::visuals::money_color), money_str.c_str());
			count++;
		}
	}
	
}


std::map<std::string, std::string> weaponIconByName = {
	{"C_DEagle",               "A"},
	{"C_WeaponElite",          "B"},
	{"C_WeaponFiveSeven",      "C"},
	{"C_WeaponGlock",          "D"},
	{"C_AK47",                 "W"},
	{"C_WeaponAug",            "U"},
	{"C_WeaponAWP",            "Z"},
	{"C_WeaponFamas",          "R"},
	{"C_WeaponG3SG1",          "X"},
	{"C_WeaponGalilAR",        "Q"},
	{"C_WeaponM249",           "g"},
	{"C_WeaponMAC10",          "K"},
	{"C_WeaponP90",            "P"},
	{"C_WeaponUMP45",          "L"},
	{"C_WeaponXM1014",         "b"},
	{"C_WeaponBizon",          "M"},
	{"C_WeaponMag7",           "d"},
	{"C_WeaponNegev",          "f"},
	{"C_WeaponSawedoff",       "c"},
	{"C_WeaponTec9",           "H"},
	{"C_WeaponTaser",          "h"},
	{"C_WeaponHKP2000",        "E"},
	{"C_WeaponMP7",            "N"},
	{"C_WeaponMP9",            "O"},
	{"C_WeaponNOVA",           "e"},
	{"C_WeaponP250",           "F"},
	{"C_WeaponSCAR20",         "Y"},
	{"C_WeaponSG556",          "V"},
	{"C_WeaponSSG08",          "a"},
	{"C_Flashbang",            "i"},
	{"C_HEGrenade",            "j"},
	{"C_SmokeGrenade",         "k"},
	{"C_MolotovGrenade",       "l"},
	{"C_DecoyGrenade",         "m"},
	{"C_IncendiaryGrenade",    "n"},
	{"C_WeaponM4A1",           "T"},
	{"C_WeaponCZ75a",          "I"}
};

// 根据武器类名获取图标（如 "C_WeaponCZ75a" → "I"）
std::string GetIconByClassName(const std::string& className) {
	auto it = weaponIconByName.find(className);
	if (it != weaponIconByName.end()) {
		return it->second;
	}
	return "";  // 未知类名返回问号图标
}
void type() {
	const auto client = reinterpret_cast<uintptr_t>(GetModuleHandle(L"client.dll"));
	auto Matrix = reinterpret_cast<float*>(client + cs2_dumper::offsets::client_dll::dwViewMatrix);
	if (!Matrix)
	{
		return;
	}
	ImDrawList* DrawList = ImGui::GetBackgroundDrawList();
	static const float w = ImGui::GetIO().DisplaySize.x;
	static const float h = ImGui::GetIO().DisplaySize.y;
	int count = 0;
	for (int i = 0; i < 1024; i++) {
		auto player_co = GetBaseEntity(i, client);
		if (!player_co)
			continue;
		const uintptr_t m_pEntity_offset = cs2_dumper::schemas::client_dll::CEntityInstance::m_pEntity;
		auto entity_identity = *reinterpret_cast<uintptr_t*>(player_co + m_pEntity_offset);
		if (!entity_identity) continue;

		// 步骤2：获取 m_pClassInfo（相对于 c_entity_identity 的偏移 0x08）
		auto class_info = *reinterpret_cast<uintptr_t*>(entity_identity + 0x08);
		if (!class_info) continue;

		// 步骤3：读取 unk1（class_info + 0x30）
		auto unk1 = *reinterpret_cast<uintptr_t*>(class_info + 0x30);
		if (!unk1) continue;

		// 步骤4：读取 unk2（unk1 + 0x08），指向类名字符串
		auto unk2 = *reinterpret_cast<const char**>(unk1 + 0x08);  // 假设是 const char*
		if (!unk2) continue;

		// 步骤5：转换为字符串并使用
		std::string type_str(unk2);
		if (type_str == "C_C4") {
			if (cs::visuals::C4) {
				uintptr_t m_pGameSceneNode_offset = cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode;
				auto game_scene_node = *reinterpret_cast<uintptr_t*>(player_co + m_pGameSceneNode_offset);
				if (!game_scene_node) continue;  // 检查指针有效性

				// 步骤2：从 CGameSceneNode 中获取 m_vecAbsOrigin
				uintptr_t m_vecAbsOrigin_offset = cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin;
				auto scene_origin = *reinterpret_cast<Vector3*>(game_scene_node + m_vecAbsOrigin_offset);
				Vector3 C4_pos{};
				WorldToScreen(scene_origin, C4_pos, Matrix, w, h);
				ImGui::PushFont(g_font_icon);
				DrawList->AddText(g_font_icon, 24.0f, ImVec2(C4_pos.x, C4_pos.y), ImColor(cs::visuals::weaponColor), "o");

				ImGui::PopFont();  // 恢复默认字体
			}
		}
		if (type_str == "C_PlantedC4") {
			if (cs::visuals::C4) {
				auto dwGlobalVars = *reinterpret_cast<std::uintptr_t*>(client + cs2_dumper::offsets::client_dll::dwGlobalVars);
				
				auto curtime = *reinterpret_cast<float*>(dwGlobalVars + 0x30);
				
				auto bBombTicking = *reinterpret_cast<bool*>(player_co + cs2_dumper::schemas::client_dll::C_PlantedC4::m_bBombTicking);
				if (bBombTicking) {

					uintptr_t m_pGameSceneNode_offset = cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode;
					auto game_scene_node = *reinterpret_cast<uintptr_t*>(player_co + m_pGameSceneNode_offset);
					// 步骤2：从 CGameSceneNode 中获取 m_vecAbsOrigin
					uintptr_t m_vecAbsOrigin_offset = cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin;
					auto scene_origin = *reinterpret_cast<Vector3*>(game_scene_node + m_vecAbsOrigin_offset);
					auto m_flC4Blow = *reinterpret_cast<float*>(player_co + cs2_dumper::schemas::client_dll::C_PlantedC4::m_flC4Blow);
					auto blow_time = m_flC4Blow - curtime;

					auto is_defusing = *reinterpret_cast<bool*>(player_co + cs2_dumper::schemas::client_dll::C_PlantedC4::m_bBeingDefused);
					auto m_flDefuseCountDown = *reinterpret_cast<float*>(player_co + cs2_dumper::schemas::client_dll::C_PlantedC4::m_flDefuseCountDown);
					auto defuse_time = m_flDefuseCountDown - curtime;
					std::string blow_time_str = Format(u8"%.1f秒", blow_time);
					std::string defuse_time_str = Format(u8"%.1f秒", defuse_time);
					Vector3 C4_pos{};
					WorldToScreen(scene_origin, C4_pos, Matrix, w, h);
					ImGui::PushFont(g_font_icon);
					DrawList->AddText(g_font_icon, 24.0f, ImVec2(C4_pos.x, C4_pos.y), ImColor(cs::visuals::weaponColor), "o");
					ImGui::PopFont();  // 恢复默认字体
					DrawList->AddText(ImVec2(w / 4 * 3 + 50, 0), ImColor(cs::visuals::weaponColor), u8"爆炸剩余时间：");
					DrawList->AddText(ImVec2(w / 4 * 3 + 150, 0), ImColor(cs::visuals::weaponColor), blow_time_str.c_str());
					DrawList->AddText(ImVec2(w / 4 * 3 + 50, 15), ImColor(cs::visuals::weaponColor), is_defusing ? u8"炸弹状态：      正在拆除" : u8"炸弹状态：      未被拆除");
					if (is_defusing) {	
						DrawList->AddText(ImVec2(w / 4 * 3 + 50, 30), ImColor(cs::visuals::weaponColor), u8"拆除剩余时间：");
						DrawList->AddText(ImVec2(w / 4 * 3 + 150, 30), ImColor(cs::visuals::weaponColor), defuse_time_str.c_str());
					}
					
					
				}
			}
		}
		if (cs::visuals::all_weapon) {
			if (type_str == "C_AK47"
				|| type_str == "C_WeaponP90"
				|| type_str == "C_WeaponMAC10"
				|| type_str == "C_WeaponMP7"  // 去重后保留一次
				|| type_str == "C_WeaponElite"
				|| type_str == "C_WeaponGlock"
				|| type_str == "C_WeaponP250"
				|| type_str == "C_WeaponTec9"
				|| type_str == "C_DEagle"  // 去重后保留一次
				|| type_str == "C_WeaponXM1014"
				|| type_str == "C_WeaponNOVA"
				|| type_str == "C_WeaponTaser"
				|| type_str == "C_WeaponGalilAR"
				|| type_str == "C_WeaponSG556"
				|| type_str == "C_WeaponSSG08"
				|| type_str == "C_Flashbang"
				|| type_str == "C_SmokeGrenade"
				|| type_str == "C_HEGrenade"
				|| type_str == "C_MolotovGrenade"
				|| type_str == "C_DecoyGrenade"
				|| type_str == "C_WeaponAWP"
				|| type_str == "C_WeaponHKP2000"  // 去重后保留一次
				|| type_str == "C_IncendiaryGrenade"
				|| type_str == "C_WeaponAug"
				|| type_str == "C_WeaponM4A1"
				|| type_str == "C_WeaponFamas"
				|| type_str == "C_WeaponMP9"
				|| type_str == "C_WeaponFiveSeven"
				|| type_str == "C_WeaponCZ75a"
				|| type_str == "C_WeaponNegev"
				|| type_str == "C_WeaponMag7"
				|| type_str == "C_WeaponBizon"
				|| type_str == "C_WeaponUMP45"
				|| type_str == "C_WeaponSCAR20"
				|| type_str == "C_WeaponG3SG1"
				|| type_str == "C_WeaponM249"
				|| type_str == "C_WeaponSawedoff"
				) {
				uintptr_t m_pGameSceneNode_offset = cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode;
				auto game_scene_node = *reinterpret_cast<uintptr_t*>(player_co + m_pGameSceneNode_offset);
				if (!game_scene_node) continue;  // 检查指针有效性

				// 步骤2：从 CGameSceneNode 中获取 m_vecAbsOrigin
				uintptr_t m_vecAbsOrigin_offset = cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin;
				auto scene_origin = *reinterpret_cast<Vector3*>(game_scene_node + m_vecAbsOrigin_offset);
				Vector3 C4_pos{};
				WorldToScreen(scene_origin, C4_pos, Matrix, w, h);
				ImGui::PushFont(g_font_icon);
				std::string weaponIcon = GetIconByClassName(type_str);
				DrawList->AddText(g_font_icon, 16.0f, ImVec2(C4_pos.x, C4_pos.y), ImColor(cs::visuals::weaponColor), weaponIcon.c_str());

				ImGui::PopFont();  // 恢复默认字体
			}
		}
	}
}