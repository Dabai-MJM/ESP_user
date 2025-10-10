#pragma once
#include <Windows.h>
#include <string>
namespace cs {
	namespace visuals {
		inline bool box = false;
		inline const char* BoxTypeOptions[] = {
		u8"完全方框",   // 选项0
		u8"四角方框",   // 选项1
		};
		// 关键修改：默认选中索引设为 -1（无效值，默认不选任何选项）
		inline int SelectedBoxType = -1;
		// 选项总数（不变）
		inline const int BoxTypeCount = sizeof(BoxTypeOptions) / sizeof(BoxTypeOptions[0]);
		inline bool OutLine = false;
		inline ImVec4 boxColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

		inline bool FilledVisBox = false;
		inline bool FilledBox = false;

		inline ImVec4 PathFillConvex = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);//白色
		inline ImVec4 FillConvex = ImVec4(1.0f, 0.0f, 0.0f, 0.5f);//红色
		inline ImVec4 FillConvex2 = ImVec4(1.0f, 0.0f, 0.0f, 0.5f);//红色
		inline bool boxMultiColor = false;
		inline  bool bone = false;
		inline ImVec4 boneColor = ImVec4(1.0f, 0.0f, 0.0f, 0.5f);//红色
		inline ImVec4 boneVisColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);//绿色
		inline  bool health = false;
		inline ImVec4 healthColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);//白色
		inline  bool healthbar = false;
		inline  bool armor = false;
		inline  bool armorNum = false;
		

		inline  bool AmmoBar = false;
		inline  bool AmmoNum = false;
		inline  bool HeadCircle = false;
		inline ImVec4 HeadCircleColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); 
		inline  bool distance = false;
		inline ImVec4 distanceColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);//白色
		inline  bool name = false;
		inline ImVec4 nameColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);//白色
		inline  bool weapon = false;
		inline ImVec4 weaponColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);//白色
		inline  bool aimbot = false;
		inline  bool foundTarget = false;
		inline  bool isTarget = false;
		inline  bool toggleMode = true;
		inline  bool aimbotActive = false;
		inline  bool Aimbot = false;
		inline  bool aimbotToggle = false;
		inline  bool prevHotkeyState = false;
		inline  bool waitingForKeybind = false;
		inline  int aimbotHotkey = VK_RBUTTON;
		inline  int current_item = 0; // 当前选中的索引
		inline  float FOV = 50.0f;
		inline  float Smoothness = 1.0f;
		inline  bool help = false;
		inline  bool loadnade = false;
		inline bool RecordHelper = false;
		inline bool waitingForKeybind_nade = false;
		inline int aimbotHotkey_nade = VK_MBUTTON;
		inline bool autoaim = false;
	}
	
	
}

void draw_Menu();
std::string GetWeaponName(int weaponID);
std::string GetWeaponIcon(int weaponID);