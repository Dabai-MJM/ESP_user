#pragma once
#include <basetsd.h>
#include <cstdint>
#include <optional>
#include "../main/memhv.h"
#include "mouse.h"
#include "../imgui_d11/imgui.h"
#include "vector.h"

struct Vector3
{
    float x, y, z;

    // 默认构造函数
    Vector3() : x(0), y(0), z(0) {}

    // 带参数的构造函数
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator+(Vector3 d);
    Vector3 operator-(Vector3 d);
    Vector3 operator*(Vector3 d);
    Vector3 operator*(float d);
    Vector3& operator-=(Vector3 d);
    Vector3& operator+=(Vector3 d);

    // Lerp 方法声明
    Vector3 Lerp(const Vector3& target, float t) const;

    Vector3 Normalized() const;
    float Dot(const Vector3& other) const;
    float Length() const;
    float Length2DSqr() const;
    float Length2D() const;
    bool IsVectorEmpty() const;

    Vector3 AnglesToVectors(Vector3* pForward, Vector3* pRight = nullptr, Vector3* pUp = nullptr) const;

    Vector3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    // 向量加向量
    Vector3 operator+(const Vector3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

};

struct Vector2 {
    float x, y;
};

struct PlayerPosition {
    Vector3 head;    // 头部位置
    Vector3 foot;    // 脚部位置
};



struct ViewMatrix {
    float matrix[4][4];
};

struct BoneJointData
{
    Vector3 Pos;
    char pad[0x14];
};

struct BoneJointPos
{
    Vector3 Pos;
    Vector3 ScreenPos;
    bool IsVisible = false;
};

struct Vec2 {
	float x;
	float y;

	Vec2() : x(0.0f), y(0.0f) {}
	Vec2(float x, float y) : x(x), y(y) {}


	bool operator< (const Vec2& other) {
		if (x < other.x) return true;
		if (x > other.x) return false;
		return y < other.y;
	}
};



uintptr_t GetPawnFromController(ULONG64 Client, int index);
ViewMatrix GetGameViewMatrix(ULONG64 Client);  // 改为更具描述性的名称

uintptr_t GetBaseEntity(ULONG64 Client, int index);
bool WorldToScreen_help(const Vector3& worldPosition, Vector3& screenPosition, const ViewMatrix& viewMatrix, int screenWidth, int screenHeight);

PlayerPosition GetPlayerPosition(ULONG64 Client, uintptr_t playerController);
std::optional<Vector3> GetEyePos_help(uintptr_t addr) noexcept;


int GetPlayerHealth(ULONG64 Client, uintptr_t playerController);

std::string GetName(uintptr_t playerController);

int GetEntityVisible(uintptr_t EntityPawn);

float clamp(float value, float min, float max);


void moveMouseByOffset(int offsetX, int offsetY);


void startAimbot(Vector3 headScreen,float smoothness);

void drawFovCircle(float radius);

float getDistanceToCenter(const Vector3& screenPos);
std::string GetMapName(ULONG64 Server);

std::string GetWeaponName_help( uintptr_t playerPawn);
Vector3 AngleToWorldPosition(const Vector3& eyePos, const QAngle& viewAngles, float distance = 1000.0f);
