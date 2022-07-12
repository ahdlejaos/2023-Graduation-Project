#pragma once
#include "pch.hpp"
#include "Arithmetics.inl"

class Transformer
{
public:
	Transformer();
	Transformer(const Transformer&) = default;
	Transformer(Transformer&&) = default;
	Transformer& operator=(const Transformer&) = default;
	Transformer& operator=(Transformer&&) = default;
	~Transformer();

	inline Transformer& SetMatrix(const XMFLOAT4X4& mat);
	inline Transformer& SetMatrix(XMFLOAT4X4&& mat);
	inline Transformer& SetScale(float x, float y, float z);
	inline Transformer& SetScale(std::span<float, 3> scl);
	inline Transformer& SetPosition(float x, float y, float z);
	inline Transformer& SetPosition(const XMFLOAT3& pos);
	inline Transformer& SetPosition(XMFLOAT3&& pos);
	inline Transformer& SetPosition(std::span<float, 3> pos);
	inline Transformer& SetRotation(const XMFLOAT4X4& tfrm);
	inline Transformer& SetRotation(XMFLOAT4X4&& tfrm);

	inline Transformer& Translate(float x, float y, float z);
	inline Transformer& Translate(const XMFLOAT3& shift);
	inline Transformer& Translate(XMFLOAT3&& shift);
	inline Transformer& Translate(std::span<float, 3> shift);
	inline Transformer& Move(const XMFLOAT3& dir, float distance);
	inline Transformer& Move(XMFLOAT3&& dir, float distance);
	inline Transformer& Move(std::span<float, 3> dir, float distance);
	inline Transformer& Move(std::span<float, 4> vector);
	inline Transformer& MoveStrafe(float distance);
	inline Transformer& MoveForward(float distance);
	inline Transformer& MoveUp(float distance);

	inline Transformer& Rotate(const XMFLOAT4X4& tfrm);
	inline Transformer& Rotate(XMFLOAT4X4&& tfrm);
	inline Transformer& Rotate(float pitch, float yaw, float roll);
	inline Transformer& Rotate(const XMFLOAT3& axis, float angle);
	inline Transformer& Rotate(const XMFLOAT4& quaternion);
	inline Transformer& Rotate(XMFLOAT4&& quaternion);

	inline Transformer& LookTo(const XMFLOAT3& look, const XMFLOAT3& up);
	inline Transformer& LookTo(XMFLOAT3&& look, const XMFLOAT3& up);
	inline Transformer& LookTo(const XMFLOAT3& look, XMFLOAT3&& up);
	inline Transformer& LookTo(XMFLOAT3&& look, XMFLOAT3&& up);
	inline Transformer& LookAt(const XMFLOAT3& look, const XMFLOAT3& up);
	inline Transformer& LookAt(XMFLOAT3&& look, const XMFLOAT3& up);
	inline Transformer& LookAt(const XMFLOAT3& look, XMFLOAT3&& up);
	inline Transformer& LookAt(XMFLOAT3&& look, XMFLOAT3&& up);

	inline XMFLOAT4X4& GetMatrix()&;
	inline const XMFLOAT4X4& GetMatrix() const&;
	inline XYZWrapper& GetRight()&;
	inline XYZWrapper& GetUp()&;
	inline XYZWrapper& GetLook()&;
	inline XYZWrapper& GetPosition()&;
	inline const XYZWrapper& GetRight() const&;
	inline const XYZWrapper& GetUp() const&;
	inline const XYZWrapper& GetLook() const&;
	inline const XYZWrapper& GetPosition() const&;

	inline XMFLOAT4X4 GetMatrix()&&;
	inline XMFLOAT3 GetRight()&&;
	inline XMFLOAT3 GetUp()&&;
	inline XMFLOAT3 GetLook()&&;
	inline XMFLOAT3 GetPosition()&&;

	XMFLOAT4X4 myMatrix;
	XYZWrapper myRight;
	XYZWrapper myUp;
	XYZWrapper myLook;
	XYZWrapper myPosition;

	static constexpr XMFLOAT3 Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	static constexpr XMFLOAT3 Forward = XMFLOAT3(0.0f, 0.0f, 1.0f);
	static constexpr XMFLOAT3 Right = XMFLOAT3(1.0f, 0.0f, 0.0f);

private:
	bool Updated;
};

Transformer::Transformer()
	: myMatrix(Matrix4x4::Identity())
	, myRight(myMatrix._11, myMatrix._12, myMatrix._13)
	, myUp(myMatrix._21, myMatrix._22, myMatrix._23)
	, myLook(myMatrix._31, myMatrix._32, myMatrix._33)
	, myPosition(myMatrix._41, myMatrix._42, myMatrix._43)
	, Updated(false)
{
	myRight = Transformer::Right;
	myUp = Transformer::Up;
	myLook = Transformer::Forward;
}

Transformer::~Transformer()
{}

inline Transformer& Transformer::SetMatrix(const XMFLOAT4X4& mat)
{
	myMatrix = mat;

	return *this;
}

inline Transformer& Transformer::SetMatrix(XMFLOAT4X4&& mat)
{
	myMatrix = std::forward<XMFLOAT4X4>(mat);

	return *this;
}

inline Transformer& Transformer::SetScale(float x, float y, float z)
{
	myMatrix = Matrix4x4::Multiply(XMMatrixScaling(x, y, z), myMatrix);

	return *this;
}

inline Transformer& Transformer::SetScale(std::span<float, 3> scl)
{
	myMatrix = Matrix4x4::Multiply(XMMatrixScaling(scl[0], scl[1], scl[2]), myMatrix);

	return *this;
}

inline Transformer& Transformer::SetPosition(float x, float y, float z)
{
	myPosition.x = x;
	myPosition.y = y;
	myPosition.z = z;

	return *this;
}

inline Transformer& Transformer::SetPosition(const XMFLOAT3& pos)
{
	myPosition = pos;

	return *this;
}

inline Transformer& Transformer::SetPosition(XMFLOAT3&& pos)
{
	myPosition = std::forward<XMFLOAT3>(pos);

	return *this;
}

inline Transformer& Transformer::SetPosition(std::span<float, 3> pos)
{
	myPosition = pos;

	return *this;
}

inline Transformer& Transformer::SetRotation(const XMFLOAT4X4& tfrm)
{
	myRight.x = tfrm._11;
	myRight.y = tfrm._12;
	myRight.z = tfrm._13;

	myUp.x = tfrm._21;
	myUp.y = tfrm._22;
	myUp.z = tfrm._23;

	myLook.x = tfrm._31;
	myLook.y = tfrm._32;
	myLook.z = tfrm._33;

	return *this;
}

inline Transformer& Transformer::SetRotation(XMFLOAT4X4&& tfrm)
{
	myRight.x = std::forward<float>(tfrm._11);
	myRight.y = std::forward<float>(tfrm._12);
	myRight.z = std::forward<float>(tfrm._13);

	myUp.x = std::forward<float>(tfrm._21);
	myUp.y = std::forward<float>(tfrm._22);
	myUp.z = std::forward<float>(tfrm._23);

	myLook.x = std::forward<float>(tfrm._31);
	myLook.y = std::forward<float>(tfrm._32);
	myLook.z = std::forward<float>(tfrm._33);

	return *this;
}

inline Transformer& Transformer::Translate(float x, float y, float z)
{
	myPosition.x += x;
	myPosition.y += y;
	myPosition.z += z;

	return *this;
}

inline Transformer& Transformer::Translate(const XMFLOAT3& shift)
{
	myPosition.x += shift.x;
	myPosition.y += shift.y;
	myPosition.z += shift.z;

	return *this;
}

inline Transformer& Transformer::Translate(XMFLOAT3&& shift)
{
	myPosition.x += std::forward<float>(shift.x);
	myPosition.y += std::forward<float>(shift.y);
	myPosition.z += std::forward<float>(shift.z);

	return *this;
}

inline Transformer& Transformer::Translate(std::span<float, 3> shift)
{
	myPosition.x += shift[0];
	myPosition.y += shift[1];
	myPosition.z += shift[2];

	return *this;
}

inline Transformer& Transformer::Move(const XMFLOAT3& dir, float distance)
{
	return Translate(Vector3::ScalarProduct(dir, distance));
}

inline Transformer& Transformer::Move(XMFLOAT3&& dir, float distance)
{
	return Translate(Vector3::ScalarProduct(std::forward<XMFLOAT3>(dir), distance));
}

inline Transformer& Transformer::Move(std::span<float, 3> dir, float distance)
{
	return Translate(Vector3::ScalarProduct(XMFLOAT3(dir[0], dir[1], dir[2]), distance));
}

inline Transformer& Transformer::Move(std::span<float, 4> vector)
{
	return Translate(Vector3::ScalarProduct(XMFLOAT3(vector[0], vector[1], vector[2]), vector[3]));
}

inline Transformer& Transformer::MoveStrafe(float distance)
{
	return Move(XMFLOAT3(GetRight()), distance);
}

inline Transformer& Transformer::MoveForward(float distance)
{
	return Move(XMFLOAT3(GetLook()), distance);
}

inline Transformer& Transformer::MoveUp(float distance)
{
	return Move(XMFLOAT3(GetUp()), distance);
}

inline Transformer& Transformer::Rotate(const XMFLOAT4X4& tfrm)
{
	myRight.x += tfrm._11;
	myRight.y += tfrm._12;
	myRight.z += tfrm._13;

	myUp.x += tfrm._21;
	myUp.y += tfrm._22;
	myUp.z += tfrm._23;

	myLook.x += tfrm._31;
	myLook.y += tfrm._32;
	myLook.z += tfrm._33;

	return *this;
}

inline Transformer& Transformer::Rotate(XMFLOAT4X4&& tfrm)
{
	myRight.x += std::forward<float>(tfrm._11);
	myRight.y += std::forward<float>(tfrm._12);
	myRight.z += std::forward<float>(tfrm._13);

	myUp.x += std::forward<float>(tfrm._21);
	myUp.y += std::forward<float>(tfrm._22);
	myUp.z += std::forward<float>(tfrm._23);

	myLook.x += std::forward<float>(tfrm._31);
	myLook.y += std::forward<float>(tfrm._32);
	myLook.z += std::forward<float>(tfrm._33);

	return *this;
}

inline Transformer& Transformer::Rotate(float pitch, float yaw, float roll)
{
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationYawPitchRoll(pitch, yaw, roll);

	myMatrix = Matrix4x4::Multiply(mtxRotate, myMatrix);

	myRight = Vector3::Normalize(XMFLOAT3(myRight));
	myUp = Vector3::Normalize(XMFLOAT3(myUp));
	myLook = Vector3::Normalize(XMFLOAT3(myLook));

	return *this;
}

inline Transformer& Transformer::Rotate(const XMFLOAT3& axis, float angle)
{
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationAxis(axis, angle);

	myMatrix = Matrix4x4::Multiply(mtxRotate, myMatrix);

	myRight = Vector3::Normalize(XMFLOAT3(myRight));
	myUp = Vector3::Normalize(XMFLOAT3(myUp));
	myLook = Vector3::Normalize(XMFLOAT3(myLook));

	return *this;
}

inline Transformer& Transformer::Rotate(const XMFLOAT4& quaternion)
{
	auto mid = XMLoadFloat4(&quaternion);
	auto mtxRotate = XMMatrixRotationQuaternion(mid);
	myMatrix = Matrix4x4::Multiply(mtxRotate, myMatrix);

	return *this;
}

inline Transformer& Transformer::Rotate(XMFLOAT4&& quaternion)
{
	auto mid = XMLoadFloat4(std::forward<XMFLOAT4*>(&quaternion));
	auto mtxRotate = XMMatrixRotationQuaternion(mid);
	myMatrix = Matrix4x4::Multiply(mtxRotate, myMatrix);

	return *this;
}

inline Transformer& Transformer::LookTo(const XMFLOAT3& look, const XMFLOAT3& up)
{
	return LookTo(XMFLOAT3(look), XMFLOAT3(up));
}

inline Transformer& Transformer::LookTo(XMFLOAT3&& look, const XMFLOAT3& up)
{
	return LookTo(std::forward<XMFLOAT3>(look), XMFLOAT3(up));
}

inline Transformer& Transformer::LookTo(const XMFLOAT3& look, XMFLOAT3&& up)
{
	return LookTo(XMFLOAT3(look), std::forward<XMFLOAT3>(up));
}

inline Transformer& Transformer::LookTo(XMFLOAT3&& look, XMFLOAT3&& up)
{
	const auto&& view = Matrix4x4::LookToLH(XMFLOAT3(GetPosition()), std::forward<XMFLOAT3>(look), std::forward<XMFLOAT3>(up));

	myMatrix._11 = view._11; myMatrix._12 = view._21; myMatrix._13 = view._31;
	myMatrix._21 = view._12; myMatrix._22 = view._22; myMatrix._23 = view._32;
	myMatrix._31 = view._13; myMatrix._32 = view._23; myMatrix._33 = view._33;

	myRight = Vector3::Normalize(XMFLOAT3(myRight));
	myUp = Vector3::Normalize(XMFLOAT3(myUp));
	myLook = Vector3::Normalize(XMFLOAT3(myLook));

	return *this;
}

inline Transformer& Transformer::LookAt(const XMFLOAT3& look, const XMFLOAT3& up)
{
	return LookAt(XMFLOAT3(look), XMFLOAT3(up));
}

inline Transformer& Transformer::LookAt(XMFLOAT3&& look, const XMFLOAT3& up)
{
	return LookAt(std::forward<XMFLOAT3>(look), XMFLOAT3(up));
}

inline Transformer& Transformer::LookAt(const XMFLOAT3& look, XMFLOAT3&& up)
{
	return LookAt(XMFLOAT3(look), std::forward<XMFLOAT3>(up));
}

inline Transformer& Transformer::LookAt(XMFLOAT3&& look, XMFLOAT3&& up)
{
	const auto&& view = Matrix4x4::LookAtLH(XMFLOAT3(GetPosition()), std::forward<XMFLOAT3>(look), std::forward<XMFLOAT3>(up));

	myMatrix._11 = view._11; myMatrix._12 = view._21; myMatrix._13 = view._31;
	myMatrix._21 = view._12; myMatrix._22 = view._22; myMatrix._23 = view._32;
	myMatrix._31 = view._13; myMatrix._32 = view._23; myMatrix._33 = view._33;

	myRight = Vector3::Normalize(XMFLOAT3(myRight));
	myUp = Vector3::Normalize(XMFLOAT3(myUp));
	myLook = Vector3::Normalize(XMFLOAT3(myLook));

	return *this;
}

inline XMFLOAT4X4& Transformer::GetMatrix()&
{
	return myMatrix;
}

inline const XMFLOAT4X4& Transformer::GetMatrix() const&
{
	return myMatrix;
}

inline XYZWrapper& Transformer::GetRight()&
{
	return myRight;
}

inline XYZWrapper& Transformer::GetUp()&
{
	return myUp;
}

inline XYZWrapper& Transformer::GetLook()&
{
	return myLook;
}

inline XYZWrapper& Transformer::GetPosition()&
{
	return myPosition;
}

inline const XYZWrapper& Transformer::GetRight() const&
{
	return myRight;
}

inline const XYZWrapper& Transformer::GetUp() const&
{
	return myUp;
}

inline const XYZWrapper& Transformer::GetLook() const&
{
	return myLook;
}

inline const XYZWrapper& Transformer::GetPosition() const&
{
	return myPosition;
}

inline XMFLOAT4X4 Transformer::GetMatrix()&&
{
	return myMatrix;
}

inline XMFLOAT3 Transformer::GetRight()&&
{
	return XMFLOAT3(myRight);
}

inline XMFLOAT3 Transformer::GetUp()&&
{

	return XMFLOAT3(myUp);
}

inline XMFLOAT3 Transformer::GetLook()&&
{

	return XMFLOAT3(myLook);
}

inline XMFLOAT3 Transformer::GetPosition()&&
{

	return XMFLOAT3(myPosition);
}
