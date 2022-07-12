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

	inline XMFLOAT4X4& GetMatrix() &;
	inline const XMFLOAT4X4& GetMatrix() const&;
	inline XYZWrapper& GetRight() &;
	inline XYZWrapper& GetUp() &;
	inline XYZWrapper& GetLook() &;
	inline XYZWrapper& GetPosition() &;
	inline const XYZWrapper& GetRight() const&;
	inline const XYZWrapper& GetUp() const&;
	inline const XYZWrapper& GetLook() const&;
	inline const XYZWrapper& GetPosition() const&;

	inline XMFLOAT4X4 GetMatrix() const&&;
	inline XMFLOAT3 GetRight() &&;
	inline XMFLOAT3 GetUp() &&;
	inline XMFLOAT3 GetLook() &&;
	inline XMFLOAT3 GetPosition() &&;

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
}

inline Transformer& Transformer::SetScale(std::span<float, 3> scl)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::SetPosition(float x, float y, float z)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::SetPosition(const XMFLOAT3& pos)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::SetPosition(XMFLOAT3&& pos)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::SetPosition(std::span<float, 3> pos)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::SetRotation(const XMFLOAT4X4& tfrm)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::SetRotation(XMFLOAT4X4&& tfrm)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Translate(float x, float y, float z)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Translate(const XMFLOAT3& shift)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Translate(XMFLOAT3&& shift)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Translate(std::span<float, 3> shift)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Move(const XMFLOAT3& dir, float distance)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Move(XMFLOAT3&& dir, float distance)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Move(std::span<float, 3> dir, float distance)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Move(std::span<float, 4> vector)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::MoveStrafe(float distance)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::MoveForward(float distance)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::MoveUp(float distance)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Rotate(const XMFLOAT4X4& tfrm)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Rotate(XMFLOAT4X4&& tfrm)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Rotate(float pitch, float yaw, float roll)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Rotate(const XMFLOAT3& axis, float angle)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Rotate(const XMFLOAT4& quaternion)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::Rotate(XMFLOAT4&& quaternion)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::LookTo(const XMFLOAT3& look, const XMFLOAT3& up)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::LookTo(XMFLOAT3&& look, const XMFLOAT3& up)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::LookTo(const XMFLOAT3& look, XMFLOAT3&& up)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::LookTo(XMFLOAT3&& look, XMFLOAT3&& up)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::LookAt(const XMFLOAT3& look, const XMFLOAT3& up)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::LookAt(XMFLOAT3&& look, const XMFLOAT3& up)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::LookAt(const XMFLOAT3& look, XMFLOAT3&& up)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline Transformer& Transformer::LookAt(XMFLOAT3&& look, XMFLOAT3&& up)
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline XMFLOAT4X4& Transformer::GetMatrix()&
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline const XMFLOAT4X4& Transformer::GetMatrix() const&
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline XYZWrapper& Transformer::GetRight()&
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline XYZWrapper& Transformer::GetUp()&
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline XYZWrapper& Transformer::GetLook()&
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline XYZWrapper& Transformer::GetPosition()&
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline const XYZWrapper& Transformer::GetRight() const&
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline const XYZWrapper& Transformer::GetUp() const&
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline const XYZWrapper& Transformer::GetLook() const&
{
	// // O: 여기에 return 문을 삽입합니다.
}

inline const XYZWrapper& Transformer::GetPosition() const&
{
	// // O: 여기에 return 문을 삽입합니다.
}
