#pragma once
#include "Transformer.inl"

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

	void SetOriginalCollider(const shared_ptr<BoundingOrientedBox>& box);
	void SetOriginalCollider(shared_ptr<BoundingOrientedBox>&& box);
	void Attach(GameObject* pChild);
	void BuildCollider();

	virtual void Awake(srv::Room* room, srv::Session* owner);
	void Animate(float delta_time);
	void Animate(float delta_time, const XMFLOAT4X4& parent);
	virtual void Update(float delta_time);
	void UpdateTransform();
	void UpdateTransform(const XMFLOAT4X4& parent);
	void UpdateTransform(XMFLOAT4X4&& parent);
	void EnumerateTransforms();
	void EnumerateTransforms(const XMFLOAT4X4& parent);
	void EnumerateTransforms(XMFLOAT4X4&& parent);
	void UpdateCollider();
	virtual void Release();

	bool CheckCollisionWith(GameObject* other) const;
	virtual void CollideWith(GameObject* other);

	void OnTransformUpdate();

	virtual constexpr srv::ObjectTags GetTag() const noexcept;

	void SetMatrix(const XMFLOAT4X4& mat);
	void SetMatrix(XMFLOAT4X4&& mat);
	void SetScale(float x, float y, float z);
	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& pos);
	void SetPosition(XMFLOAT3&& pos);
	void SetRotation(const XMFLOAT4X4& tfrm);
	void SetRotation(XMFLOAT4X4&& tfrm);

	void Translate(float x, float y, float z);
	void Translate(const XMFLOAT3& shift);
	void Translate(XMFLOAT3&& shift);
	void Move(const XMFLOAT3& dir, float distance);
	void Move(XMFLOAT3&& dir, float distance);
	void MoveStrafe(float distance);
	void MoveForward(float distance);
	void MoveUp(float distance);

	void Rotate(const XMFLOAT4X4& tfrm);
	void Rotate(XMFLOAT4X4&& tfrm);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(const XMFLOAT3& axis, float angle);
	void Rotate(const XMFLOAT4& quaternion);
	void Rotate(XMFLOAT4&& quaternion);

	void LookTo(const XMFLOAT3& look, const XMFLOAT3& up);
	void LookAt(const XMFLOAT3& look, const XMFLOAT3& up);

	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetLook() const;
	XMFLOAT3 GetUp() const;
	XMFLOAT3 GetRight() const;
	const GameObject* FindFrame(const char* name) const;
	GameObject* FindFrame(const char* name);
	const GameObject* GetParent() const;
	GameObject* GetParent();
	UINT GetMeshType() const;

	void PrintFrameInfo() const;
	void PrintFrameInfo(const GameObject* parent) const;

	std::string myName;

	Transformer localTransform;
	Transformer worldTransform;
	XMFLOAT4X4& localMatrix;
	XMFLOAT4X4& worldMatrix;
	bool isTransformModified;

	GameObject* m_pParent = NULL;
	GameObject* myChild = NULL;
	GameObject* mySibling = NULL;

	BoundingOrientedBox staticCollider;
	BoundingOrientedBox myCollider;
};
