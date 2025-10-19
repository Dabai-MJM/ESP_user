#pragma once
#include "../imgui_d11/imgui.h"
#include "../utils/Vector.h"
#include <vector>
#include "math.h"
uintptr_t GetBaseEntity(int index, uintptr_t client);
uintptr_t GetBaseEntityFromHandle(uint32_t uHandle, uintptr_t client);
void draw_esp();

namespace Bone_Base {

	enum BoneIndex {
		head = 6,        //头部
		neck_0 = 5,		 //颈部	
		spine_1 = 4,	 //脊椎1
		spine_2 = 2,	 //脊椎2
		pelvis = 0,		 //盆骨
		arm_upper_L = 8, //左上臂
		arm_lower_L = 9, //左前臂
		hand_L = 10,	 //左手
		arm_upper_R = 13,//右上臂
		arm_lower_R = 14,//右前臂
		hand_R = 15,	 //右手
		leg_upper_L = 22,//左大腿
		leg_lower_L = 23,//左小腿
		ankle_L = 24,	 //左脚踝
		leg_upper_R = 25,//右大腿
		leg_lower_R = 26,//右小腿
		ankle_R = 27,	 //右脚踝
	};

	
}

//取骨骼坐标
Vector3 BonePos(uintptr_t addr, int32_t index);
//全身骨骼绘制
void Bone_Start(uintptr_t pawn, ImColor BoneColor, float* Matrix);
//骨骼绘画列表的连线
void DrawLine(std::vector<Vector3> list, ImColor Color, float* Matrix);

inline std::vector<Vector3>BoneDrawList{};

struct Matrix4x4
{
	float m[4][4];
}; inline Matrix4x4* Matrix = new Matrix4x4();

struct TraceHitboxData_t
{
public:
	char pad_01[0x38];
	int m_nHitGroup;
	char pad_02[0x4];
	int m_nHitboxId;
};
static_assert(sizeof(TraceHitboxData_t) == 0x44);

class CTraceRay
{
public:
	char Pad[0x80];
};

class CTraceFilter
{
	char pad_01[0x8];
public:
	int64_t nTraceMask;
	int64_t arrUnknown[2];
	int32_t arrSkipHandles[4];
	int16_t arrCollisions[7];
	int16_t nUnknown2;
	uint8_t nUnknown3;
	uint8_t nUnknown4;
	uint8_t nUnknown5;
};

class CGameTrace {
public:

	void* pSurface;
	void* pHitEntity;
	TraceHitboxData_t* pHitboxData;
	char pad_01[0x38];
	uint32_t nContents;
	char pad_02[0x24];
	Vector3 vecStart;
	Vector3 vecEnd;
	Vector3 vecNormal;
	Vector3 vecPosition;
	char pad_03[0x4];
	float flFraction;
	char pad_04[0x6];
	bool bStartSolid;
	char pad_05[0x4D];

	bool DidHit() const noexcept {
		return (this->flFraction < 1.0f || bStartSolid);
	}

	bool IsVisible() const noexcept {
		return this->flFraction > 0.9f;
	}
};
