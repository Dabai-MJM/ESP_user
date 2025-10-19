#include <cmath> 
#include "Vector.h"
#include "qangle.h"
#include <algorithm>
#include <hacking/math.h>
#include "math.h"
Vector3 Vector3::operator+(Vector3 d)
{
    return { x + d.x, y + d.y, z + d.z };
}

Vector3 Vector3::operator-(Vector3 d)
{
    return { x - d.x, y - d.y, z - d.z };
}

Vector3 Vector3::operator*(Vector3 d)
{
    return { x * d.x, y * d.y, z * d.z };
}

Vector3 Vector3::operator*(float d)
{
    return { x * d, y * d, z * d };
}

Vector3& Vector3::operator-=(Vector3 d) {
    Vector3 vecRes = *this - d;
    *this = vecRes;
    return *this;
}

Vector3& Vector3::operator+=(Vector3 d) {
    Vector3 vecRes = *this + d;
    *this = vecRes;
    return *this;
}

Vector4 Vector4::operator+(Vector4 d)
{
    return { x + d.x, y + d.y, w + d.w, h + d.h };
}

Vector4 Vector4::operator-(Vector4 d)
{
    return { x - d.x, y - d.y, w - d.w, h - d.h };
}

Vector4 Vector4::operator*(Vector4 d)
{
    return { x * d.x, y * d.y, w * d.w, h * d.h };
}

Vector4 Vector4::operator*(float d)
{
    return { x * d, y * d, w * d, h * d};
}

// 计算向量的长度
float Vector3::Length() const {
    return std::sqrt(x * x + y * y + z * z);
}

float Vector3::Length2DSqr() const {
    return (x * x + y * y);
}

float Vector3::Length2D() const {
    return std::sqrtf(Length2DSqr());
}

// 归一化向量
Vector3 Vector3::Normalized() const {
    float length = Length();
    if (length == 0) return Vector3(0, 0, 0); // 避免除以零
    return Vector3(x / length, y / length, z / length);
}

float Vector3::Dot(const Vector3& other) const {
    return x * other.x + y * other.y + z * other.z;
}


Vector3 Vector3::Lerp(const Vector3& target, float t) const
{
    return Vector3(
        this->x + (target.x - this->x) * t,
        this->y + (target.y - this->y) * t,
        this->z + (target.z - this->z) * t
    );
}

bool Vector3::IsVectorEmpty() const {
    return x == 0.0f && y == 0.0f && z == 0.0f;
}

Vector3 Vector3::AnglesToVectors(Vector3* pForward, Vector3* pRight, Vector3* pUp) const {
    float flPitchSin, flPitchCos, flYawSin, flYawCos, flRollSin, flRollCos = 0.f;
    DirectX::XMScalarSinCos(&flPitchSin, &flPitchCos, Deg2Rad(this->x));
    DirectX::XMScalarSinCos(&flYawSin, &flYawCos, Deg2Rad(this->y));
    DirectX::XMScalarSinCos(&flRollSin, &flRollCos, Deg2Rad(this->z));
    if (pForward) {
        pForward->x = flPitchCos * flYawCos;
        pForward->y = flPitchCos * flYawSin;
        pForward->z = -flPitchSin;
    }

    if (pRight) {
        pRight->x = (-flRollSin * flPitchSin * flYawCos) + (-flRollCos * -flYawSin);
        pRight->y = (-flRollSin * flPitchSin * flYawSin) + (-flRollCos * flYawCos);
        pRight->z = (-flRollSin * flPitchCos);
    }

    if (pUp) {
        pUp->x = (flRollCos * flPitchSin * flYawCos) + (-flRollSin * -flYawSin);
        pUp->y = (flRollCos * flPitchSin * flYawSin) + (-flRollSin * flYawCos);
        pUp->z = (flRollCos * flPitchCos);
    }

    Vector3* Direction = pForward ? pForward : (pRight ? pRight : pUp);
    return { Direction ? Direction->x : 0.f, Direction ? Direction->y : 0.f, Direction ? Direction->z : 0.f };
}

[[nodiscard]] QAngle_t Vector_t::ToAngles() const
{
    float flPitch, flYaw;
    if (this->x == 0.0f && this->y == 0.0f)
    {
        flPitch = (this->z > 0.0f) ? 270.f : 90.f;
        flYaw = 0.0f;
    }
    else
    {
        flPitch = M_RAD2DEG(std::atan2f(-this->z, this->Length2D()));

        if (flPitch < 0.f)
            flPitch += 360.f;

        flYaw = M_RAD2DEG(std::atan2f(this->y, this->x));

        if (flYaw < 0.f)
            flYaw += 360.f;
    }

    return { flPitch, flYaw, 0.0f };
}

[[nodiscard]] QAngle_t Vector_t::CalculateViewAngle(const Vector_t& from, const Vector_t& to)
{
    float pitch, yaw;

    auto d = to - from;
    yaw = std::atan2(d.y, d.x) * 180 / MATH::_PI;

    double dist = d.Length2D();
    pitch = -std::atan2(d.z, dist) * 180 / MATH::_PI;

    return { pitch , yaw };
}