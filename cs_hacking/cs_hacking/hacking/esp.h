#pragma once
#include "../imgui_d11/imgui.h"
#include "../utils/Vector.h"
#include <vector>
#include "math.h"
uintptr_t GetBaseEntity(int index, uintptr_t client);
uintptr_t GetBaseEntityFromHandle(uint32_t uHandle, uintptr_t client);
void draw_esp();
void test();
void draw_money();
void type();
namespace Bone_Base {

	enum BoneIndex {
		head = 6,        //ͷ��
		neck_0 = 5,		 //����	
		spine_1 = 4,	 //��׵1
		spine_2 = 2,	 //��׵2
		pelvis = 0,		 //���
		arm_upper_L = 8, //���ϱ�
		arm_lower_L = 9, //��ǰ��
		hand_L = 10,	 //����
		arm_upper_R = 13,//���ϱ�
		arm_lower_R = 14,//��ǰ��
		hand_R = 15,	 //����
		leg_upper_L = 22,//�����
		leg_lower_L = 23,//��С��
		ankle_L = 24,	 //�����
		leg_upper_R = 25,//�Ҵ���
		leg_lower_R = 26,//��С��
		ankle_R = 27,	 //�ҽ���
	};

	
}

//ȡ��������
Vector3 BonePos(uintptr_t addr, int32_t index);
//ȫ���������
void Bone_Start(uintptr_t pawn, ImColor BoneColor, float* Matrix);
//�����滭�б������
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
