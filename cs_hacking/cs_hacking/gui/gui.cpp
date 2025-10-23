#include "../imgui_d11/imgui.h"
#include "gui.h"
#include <Windows.h>
#include <string>
#include <map>


std::string GetKeyName(int vk)
{
	UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC) << 16;
	char keyName[256] = { 0 };
	if (GetKeyNameTextA(scanCode, keyName, sizeof(keyName)))
		return keyName;
	else {
		// 鼠标侧键或未识别键，自定义处理
		switch (vk) {
		case VK_XBUTTON1: return u8"鼠标侧键 1（前进键）";
		case VK_XBUTTON2: return u8"鼠标侧键 2（后退键）";
		case VK_LBUTTON: return u8"鼠标左键";
		case VK_RBUTTON: return u8"鼠标右键";
		case VK_MBUTTON: return u8"鼠标中键（滚轮按下）";
		default: return u8"未知";
		}
	}
}
const char* boneNames[] = {
	u8"骨盆", //pelvis
	u8"脊柱 2 节", //spine_2
	u8"脊柱 1 节", //spine_1
	u8"颈部 0 节", //neck_0
	u8"头部", //head
	u8"左上臂", //arm_upper_L
	u8"左前臂", //arm_lower_L
	u8"左手", //hand_L
	u8"右上臂", //arm_upper_R
	u8"右前臂", //arm_lower_R
	u8"右手", //hand_R
	u8"左大腿", //leg_upper_L
	u8"左小腿", //leg_lower_L
	u8"左脚踝", //ankle_L
	u8"右大腿", //leg_upper_R
	u8"右小腿", //leg_lower_R
	u8"右脚踝", //ankle_R
};
std::map<int, std::string> weaponNames = {
	{1,u8"沙漠之鹰"},
	{2,u8"双持贝瑞塔"},
	{3,u8"FN57"},
	{4,u8"格洛克 18"},
	{7,u8"AK-47"},
	{8,u8"AUG"},
	{9,u8"AWP"},
	{10,u8"法玛斯"},
	{11,u8"G3SG1"},
	{13,u8"加利尔"},
	{14,u8"M249"},
	{17,u8"MAC-10"},
	{19,u8"P90"},
	{23,u8"MP5-SD"},
	{24,u8"UMP-45"},
	{25,u8"XM1014"},
	{26,u8"PP-野牛"},
	{27,u8"MAG-7"},
	{28,u8"内格夫"},
	{29,u8"截短霰弹枪"},
	{30,u8"Tec-9"},
	{31,u8"宙斯电击枪"},
	{32,u8"P2000"},
	{33,u8"MP7"},
	{34,u8"MP9"},
	{35,u8"Nova霰弹枪"},
	{36,u8"P250"},
	{38,u8"SCAR-20"},
	{39,u8"SG556"},
	{40,u8"SSG08"},
	{42,u8"CT阵营刀"},
	{43,u8"闪光弹"},
	{44,u8"高爆手雷"},
	{45,u8"烟雾弹"},
	{46,u8"燃烧瓶"},
	{47,u8"诱饵弹"},
	{48,u8"燃烧弹"},
	{49,u8"C4炸弹"},
	{16,u8"M4A4"},
	{61,u8"USP消音版"},
	{60,u8"M4A1消音版"},
	{63,u8"CZ75-Auto"},
	{64,u8"R8左轮手枪"},
	{59, u8"T阵营匕首" },
	{505, u8"折叠刀"},
	{506, u8"穿肠刀"},
	{507, u8"爪子刀"},
	{508, u8"M9刺刀"},
	{509, u8"战术刀"},
	{512, u8"弯刀"},
	{514, u8"鲍伊猎刀"},
	{515, u8"蝴蝶刀"},
	{516, u8"锯齿爪刀"},
	{526, u8"廓尔喀刀"},
};

std::map<int, std::string> weaponIcon = {
	{1,"A"},
	{2,"B"},
	{3,"C"},
	{4,"D"},
	{7,"W"},
	{8,"U"},
	{9,"Z"},
	{10,"R"},
	{11,"X"},
	{13,"Q"},
	{14,"g"},
	{17,"K"},
	{19,"P"},
	{23,"p"},
	{24,"L"},
	{25,"b"},
	{26,"M"},
	{27,"d"},
	{28,"f"},
	{29,"c"},
	{30,"H"},
	{31,"h"},
	{32,"E"},
	{33,"N"},
	{34,"O"},
	{35,"e"},
	{36,"F"},
	{38,"Y"},
	{39,"V"},
	{40,"a"},
	{42,"]"},
	{43,"i"},
	{44,"j"},
	{45,"k"},
	{46,"l"},
	{47,"m"},
	{48,"n"},
	{49,"o"},
	{16,"S"},
	{61,"G"},
	{60,"T"},
	{63,"I"},
	{64,"J"},
	{59, "[" },
	{505, "0"},
	{506, "3"},
	{507, "4"},
	{508, "5"},
	{509, "6"},
	{512, "2"},
	{514, "7"},
	{515, "8"},
	{516, "4"},
	{526, "5"},
};
std::string GetWeaponName(int weaponID) {
	auto it = weaponNames.find(weaponID);
	if (it != weaponNames.end()) {
		return it->second;
	}
	return "Weapon_None";
}
std::string GetWeaponIcon(int weaponID) {
	auto it = weaponIcon.find(weaponID);
	if (it != weaponIcon.end()) {
		return it->second;
	}
	return "";
}

std::string  MapName;
void draw_Menu() {
	const char* BoxTypeOptions[] = {
		u8"完全方框",   // 选项0
		u8"四角方框",   // 选项1
	};
	ImGui::Begin(u8"嘉明专属");
	{
		
		ImGui::Checkbox(u8"当前队伍(CT)", &cs::visuals::team_CT);
		ImGui::Checkbox(u8"当前队伍(T)", &cs::visuals::team_T);
		
		ImGui::Checkbox(u8"方框透视", &cs::visuals::box);
		if (cs::visuals::box)
		{
			ImGui::ColorEdit4(u8"方框可见颜色", &cs::visuals::PathFillConvex.x);
			ImGui::ColorEdit4(u8"方框不可见颜色", &cs::visuals::boxColor.x);
			ImGui::ListBox(
				"##BoxType",
				&cs::visuals::SelectedBoxType,
				BoxTypeOptions,
				cs::visuals::BoxTypeCount,
				2 // 最大显示行数
			);
			switch (cs::visuals::SelectedBoxType) {
			case 0:  // 第一个选项
				ImGui::Checkbox(u8"粗边框", &cs::visuals::OutLine);
				break;
			}
		}
		
		ImGui::Separator();
		
		ImGui::Checkbox(u8"填充方框透视", &cs::visuals::FilledBox);
		if (cs::visuals::FilledBox) {
			ImGui::Checkbox(u8"可见区分填充方框", &cs::visuals::FilledVisBox);
			ImGui::Checkbox(u8"渐变方框", &cs::visuals::boxMultiColor);
			if (cs::visuals::FilledVisBox) {
				ImGui::ColorEdit4(u8"可见颜色", &cs::visuals::PathFillConvex.x);
				ImGui::ColorEdit4(u8"不可见颜色", &cs::visuals::FillConvex.x);
			}
			if (cs::visuals::boxMultiColor) {
				ImGui::ColorEdit4(u8"渐变颜色1", &cs::visuals::FillConvex.x);
				ImGui::ColorEdit4(u8"渐变颜色2", &cs::visuals::FillConvex2.x);
			}
			
			
		}
		
		ImGui::Separator();
		ImGui::Checkbox(u8"骨骼透视", &cs::visuals::bone);
		if (cs::visuals::bone) {
			ImGui::ColorEdit4(u8"骨骼可见颜色", &cs::visuals::boneVisColor.x);
			ImGui::ColorEdit4(u8"骨骼不可见颜色", &cs::visuals::boneColor.x);
			
		}
		
		ImGui::Checkbox(u8"护甲", &cs::visuals::armor);
		if (cs::visuals::armor) {
			ImGui::Checkbox(u8"护甲值", &cs::visuals::armorNum);
		}

		ImGui::Checkbox(u8"弹药条", &cs::visuals::AmmoBar);
		if (cs::visuals::AmmoBar) {
			ImGui::Checkbox(u8"弹药值(当前弹药|总弹药)", &cs::visuals::AmmoNum);
		}
		ImGui::Checkbox(u8"血量条", &cs::visuals::healthbar);
		if (cs::visuals::healthbar) {
			ImGui::Checkbox(u8"血量", &cs::visuals::health);
			ImGui::ColorEdit4(u8"血量颜色", &cs::visuals::healthColor.x);
		}
		ImGui::Checkbox(u8"头部", &cs::visuals::HeadCircle);

		ImGui::Checkbox(u8"距离(m)", &cs::visuals::distance);
		if (cs::visuals::distance) {
			ImGui::ColorEdit4(u8"距离颜色", &cs::visuals::distanceColor.x);
		}
		ImGui::Checkbox(u8"武器名称", &cs::visuals::weapon);
		if (cs::visuals::weapon) {
			
			ImGui::Checkbox(u8"C4信息", &cs::visuals::C4);
			ImGui::Checkbox(u8"显示所有武器位置", &cs::visuals::all_weapon);
			ImGui::ColorEdit4(u8"武器图标颜色", &cs::visuals::weaponColor.x);

		}

		ImGui::Checkbox(u8"敌方金钱", &cs::visuals::money);
		if (cs::visuals::money) {
			ImGui::ColorEdit4(u8"敌方金钱显示颜色", &cs::visuals::money_color.x);
		}
		ImGui::Checkbox(u8"自瞄", &cs::visuals::aimbot);
		if (cs::visuals::aimbot) {
			ImGui::Text(u8"自瞄热键: ");
			ImGui::SameLine();
			if (ImGui::Button(u8"等待用户输入")) {
				cs::visuals::waitingForKeybind = true;
			}

			if (cs::visuals::waitingForKeybind) {
				for (int vk = 1; vk < 256; vk++) {
					if (GetAsyncKeyState(vk) & 0x8000) {
						cs::visuals::aimbotHotkey = vk;
						cs::visuals::waitingForKeybind = false;
						break;
					}
				}
			}

			ImGui::Checkbox(u8"自瞄模式(单击|长按)", &cs::visuals::toggleMode); // 勾选=切换模式，取消=长按模式
			// 显示当前热键名称（可选）
			ImGui::Text(u8"热键名称: %s", GetKeyName(cs::visuals::aimbotHotkey).c_str());



			ImGui::Combo(u8"自瞄部位", &cs::visuals::current_item, boneNames, IM_ARRAYSIZE(boneNames));


			// 显示滑块
			ImGui::PushItemWidth(200); // 设置滑动条宽度
			ImGui::SliderFloat(u8"平滑度", &cs::visuals::Smoothness, 0.0f, 10.0f);
			ImGui::SliderFloat(u8"范围", &cs::visuals::FOV, 0.0f, 500.0f);
			ImGui::PopItemWidth();
		}

		ImGui::Checkbox(u8"辅助道具投掷", &cs::visuals::help); 
		if (cs::visuals::help) {
			ImGui::Checkbox(u8"载入投掷物点位数据", &cs::visuals::loadnade);
			ImGui::Checkbox(u8"记录投掷物点位数据", &cs::visuals::RecordHelper);
			if (cs::visuals::RecordHelper) {
				ImGui::Text(u8"记录点位热键: ");
				ImGui::SameLine();
				if (ImGui::Button(u8"等待用户输入")) {
					cs::visuals::waitingForKeybind_nade = true;
				}

				if (cs::visuals::waitingForKeybind_nade) {
					for (int vk = 1; vk < 256; vk++) {
						if (GetAsyncKeyState(vk) & 0x8000) {
							cs::visuals::aimbotHotkey_nade = vk;
							cs::visuals::waitingForKeybind_nade = false;
							break;
						}
					}
				}
				ImGui::Text(u8"记录点位热键名称: %s", GetKeyName(cs::visuals::aimbotHotkey_nade).c_str());
			}

		}

		ImGui::Checkbox(u8"自瞄扳机(适用于狙)", &cs::visuals::autoaim);
	}
	ImGui::End();
}