#pragma once

class Runnable
{
public:
	constexpr Runnable() {}
	constexpr virtual ~Runnable() {}

	virtual void Awake() = 0;
	virtual void Start() = 0;
	virtual void Update(float delta_time) = 0;
	virtual void Release() = 0;
};
